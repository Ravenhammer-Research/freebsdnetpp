/**
 * @file routing/table.cpp
 * @brief Routing table implementation
 * @details Provides C++ wrapper for FreeBSD routing table operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <net/route/route_ctl.h>
#include <netinet/in.h>
#include <routing/entry.hpp>
#include <routing/table.hpp>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

namespace libfreebsdnet::routing {

  class RoutingTable::Impl {
  public:
    Impl() : socket_fd(-1), lastError_("") {
      socket_fd = socket(AF_ROUTE, SOCK_RAW, 0);
      if (socket_fd < 0) {
        lastError_ =
            "Failed to create routing socket: " + std::string(strerror(errno));
        throw std::runtime_error(lastError_);
      }
    }

    ~Impl() {
      if (socket_fd >= 0) {
        close(socket_fd);
      }
    }

    /**
     * @brief Get interface name by index using getifaddrs
     * @param index Interface index
     * @return Interface name or empty string if not found
     */
    std::string getInterfaceName(int index) const {
      struct ifaddrs *ifap, *ifa;
      
      if (getifaddrs(&ifap) != 0) {
        return "";
      }

      for (ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
          struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
          if (sdl->sdl_index == index) {
            std::string name = ifa->ifa_name;
            freeifaddrs(ifap);
            return name;
          }
        }
      }

      freeifaddrs(ifap);
      return "";
    }

    std::vector<std::unique_ptr<RoutingEntry>> getEntries() const {
      return getEntries(0); // Default FIB
    }

    std::vector<std::unique_ptr<RoutingEntry>> getEntries(int fib) const {
      std::vector<std::unique_ptr<RoutingEntry>> entries;

      // Get both IPv4 and IPv6 routes (like FreeBSD netstat does)
      std::vector<int> address_families = {AF_INET, AF_INET6};

      for (int af : address_families) {
        // Use sysctl to get routing table for specific FIB and address family
        size_t len = 0;
        int mib[] = {CTL_NET, PF_ROUTE, 0, af, NET_RT_DUMP, 0, fib};

        // Get the size first (like FreeBSD netstat does)
        if (sysctl(mib, 7, nullptr, &len, nullptr, 0) < 0) {
          // Continue with other address families if one fails
          continue;
        }

        if (len == 0) {
          continue;
        }

        // Allocate buffer
        std::vector<char> buffer(len);

        // Get the routing table (like FreeBSD netstat does)
        if (sysctl(mib, 7, buffer.data(), &len, nullptr, 0) < 0) {
          // Continue with other address families if one fails
          continue;
        }

        // Parse the routing messages
        char *ptr = buffer.data();
        char *end = ptr + len;

        while (ptr < end) {
          struct rt_msghdr *rtm = reinterpret_cast<struct rt_msghdr *>(ptr);

          if (rtm->rtm_version != RTM_VERSION) {
            ptr += rtm->rtm_msglen;
            continue;
          }

          // Create routing entry
          auto entry = parseRoutingMessage(rtm);
          if (entry) {
            entries.push_back(std::move(entry));
          }

          ptr += rtm->rtm_msglen;
        }
      }

      return entries;
    }

    std::vector<std::unique_ptr<RoutingEntry>>
    getEntries(const std::string &destination) const {
      std::vector<std::unique_ptr<RoutingEntry>> entries;

      // Get all entries first
      auto allEntries = getEntries();

      // Filter by destination
      for (auto &entry : allEntries) {
        if (entry->getDestination() == destination) {
          entries.push_back(std::move(entry));
        }
      }

      return entries;
    }

    bool addEntry(const std::string &destination, const std::string &gateway,
                  const std::string &interface, uint16_t flags) {
      return addEntry(destination, gateway, interface, flags, 0); // Default FIB
    }

    bool addEntry(const std::string &destination, const std::string &gateway,
                  const std::string &interface, uint16_t flags, int fib) {
      // Create routing message
      struct {
        struct rt_msghdr rtm;
        struct sockaddr_in dst;
        struct sockaddr_in gw;
        struct sockaddr_in mask;
        struct sockaddr_dl ifp;
      } msg;

      std::memset(&msg, 0, sizeof(msg));

      // Set up routing message header
      msg.rtm.rtm_version = RTM_VERSION;
      msg.rtm.rtm_type = RTM_ADD;
      msg.rtm.rtm_flags = flags | RTF_UP;
      msg.rtm.rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK | RTA_IFP;
      msg.rtm.rtm_pid = getpid();
      msg.rtm.rtm_seq = 1;
      // Set FIB number in routing message - use rtm_flags for FIB
      msg.rtm.rtm_flags |= (fib << 8); // Store FIB in upper bits of flags

      // Set destination
      msg.dst.sin_family = AF_INET;
      msg.dst.sin_len = sizeof(msg.dst);
      if (inet_pton(AF_INET, destination.c_str(), &msg.dst.sin_addr) != 1) {
        lastError_ = "Invalid destination address: " + destination;
        return false;
      }

      // Set gateway
      msg.gw.sin_family = AF_INET;
      msg.gw.sin_len = sizeof(msg.gw);
      if (inet_pton(AF_INET, gateway.c_str(), &msg.gw.sin_addr) != 1) {
        lastError_ = "Invalid gateway address: " + gateway;
        return false;
      }

      // Set netmask (default to /32)
      msg.mask.sin_family = AF_INET;
      msg.mask.sin_len = sizeof(msg.mask);
      msg.mask.sin_addr.s_addr = 0xFFFFFFFF;

      // Set interface
      msg.ifp.sdl_family = AF_LINK;
      msg.ifp.sdl_len = sizeof(msg.ifp);
      msg.ifp.sdl_index = if_nametoindex(interface.c_str());
      if (msg.ifp.sdl_index == 0) {
        lastError_ = "Invalid interface: " + interface;
        return false;
      }

      msg.rtm.rtm_msglen = sizeof(msg);

      // Send routing message
      if (write(socket_fd, &msg, sizeof(msg)) < 0) {
        lastError_ =
            "Failed to add routing entry: " + std::string(strerror(errno));
        return false;
      }

      return true;
    }

    bool deleteEntry(const std::string &destination,
                     const std::string &gateway) {
      // Create routing message
      struct {
        struct rt_msghdr rtm;
        struct sockaddr_in dst;
        struct sockaddr_in gw;
      } msg;

      std::memset(&msg, 0, sizeof(msg));

      // Set up routing message header
      msg.rtm.rtm_version = RTM_VERSION;
      msg.rtm.rtm_type = RTM_DELETE;
      msg.rtm.rtm_flags = RTF_UP;
      msg.rtm.rtm_addrs = RTA_DST | RTA_GATEWAY;
      msg.rtm.rtm_pid = getpid();
      msg.rtm.rtm_seq = 1;

      // Set destination
      msg.dst.sin_family = AF_INET;
      msg.dst.sin_len = sizeof(msg.dst);
      if (inet_pton(AF_INET, destination.c_str(), &msg.dst.sin_addr) != 1) {
        lastError_ = "Invalid destination address: " + destination;
        return false;
      }

      // Set gateway
      msg.gw.sin_family = AF_INET;
      msg.gw.sin_len = sizeof(msg.gw);
      if (inet_pton(AF_INET, gateway.c_str(), &msg.gw.sin_addr) != 1) {
        lastError_ = "Invalid gateway address: " + gateway;
        return false;
      }

      msg.rtm.rtm_msglen = sizeof(msg);

      // Send routing message
      if (write(socket_fd, &msg, sizeof(msg)) < 0) {
        lastError_ =
            "Failed to delete routing entry: " + std::string(strerror(errno));
        return false;
      }

      return true;
    }

    bool flush() {
      // Flush all routing entries
      struct {
        struct rt_msghdr rtm;
      } msg;

      std::memset(&msg, 0, sizeof(msg));

      msg.rtm.rtm_version = RTM_VERSION;
      msg.rtm.rtm_type = RTM_DELETE;
      msg.rtm.rtm_flags = RTF_UP;
      msg.rtm.rtm_addrs = 0;
      msg.rtm.rtm_pid = getpid();
      msg.rtm.rtm_seq = 1;
      msg.rtm.rtm_msglen = sizeof(msg);

      if (write(socket_fd, &msg, sizeof(msg)) < 0) {
        lastError_ =
            "Failed to flush routing table: " + std::string(strerror(errno));
        return false;
      }

      return true;
    }

    std::unique_ptr<RoutingEntry> getDefaultGateway() const {
      // Get all entries and find default route (0.0.0.0/0)
      auto entries = getEntries();

      for (auto &entry : entries) {
        if (entry->getDestination() == "0.0.0.0") {
          return std::move(entry);
        }
      }

      return nullptr;
    }

    bool isAccessible() const { return socket_fd >= 0; }

    std::string getLastError() const { return lastError_; }

    int getFibCount() const {
      // Get number of FIBs using sysctl (like FreeBSD netstat/route)
      int numfibs = -1;
      size_t len = sizeof(numfibs);

      if (sysctlbyname("net.fibs", &numfibs, &len, nullptr, 0) == -1) {
        return -1;
      }

      return numfibs;
    }

    int getDefaultFib() const {
      // Get default FIB using sysctl (like FreeBSD netstat/route)
      int defaultfib = -1;
      size_t len = sizeof(defaultfib);

      if (sysctlbyname("net.my_fibnum", &defaultfib, &len, nullptr, 0) == -1) {
        return -1;
      }

      return defaultfib;
    }

  private:
    int socket_fd;
    mutable std::string lastError_;

    std::string getNetmaskFromRoutingMessage(struct rt_msghdr *rtm) const {
      char *ptr = reinterpret_cast<char *>(rtm + 1);
      struct sockaddr *addr[RTAX_MAX] = {nullptr};

      // First pass: collect all addresses
      for (int i = 0; i < RTAX_MAX; i++) {
        if (rtm->rtm_addrs & (1 << i)) {
          addr[i] = reinterpret_cast<struct sockaddr *>(ptr);
          ptr = reinterpret_cast<char *>(ptr) + SA_SIZE(addr[i]);
        }
      }

      // Check if we have a netmask
      if (addr[RTAX_NETMASK] != nullptr) {
        struct sockaddr *mask_sa = addr[RTAX_NETMASK];
        if (mask_sa->sa_family == AF_INET) {
          struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(mask_sa);
          uint32_t mask = ntohl(sin->sin_addr.s_addr);
          int prefix_length = 0;
          if (mask == 0) {
            prefix_length = 0; // Default route
          } else {
            while (mask & 0x80000000) {
              prefix_length++;
              mask <<= 1;
            }
          }
          return std::to_string(prefix_length);
        } else if (mask_sa->sa_family == AF_INET6) {
          struct sockaddr_in6 *sin6 = reinterpret_cast<struct sockaddr_in6 *>(mask_sa);
          // Calculate prefix length for IPv6 like netname6 does
          int prefix_length = 0;
          for (int j = 0; j < 16; j++) {
            uint8_t byte = sin6->sin6_addr.s6_addr[j];
            if (byte == 0xff) {
              prefix_length += 8;
            } else if (byte != 0) {
              while (byte & 0x80) {
                prefix_length++;
                byte <<= 1;
              }
              break;
            }
          }
          return std::to_string(prefix_length);
        }
      }
      return "";
    }

    std::unique_ptr<RoutingEntry>
    parseRoutingMessage(struct rt_msghdr *rtm) const {
      // Complete routing message parser handling all address types

      char dst_str[INET6_ADDRSTRLEN] = {0};
      char gw_str[INET6_ADDRSTRLEN] = {0};
      char mask_str[INET6_ADDRSTRLEN] = {0};
      char mac_str[18] = {0}; // MAC address string buffer
      std::string interface = "unknown";
      std::string gateway_display = "";
      std::string netmask_display = "";
      int link_index = -1;
      int scope_id = -1;
      int prefix_length = -1;

      // Parse addresses from the message
      char *ptr = reinterpret_cast<char *>(rtm + 1);

      for (int i = 0; i < RTAX_MAX; i++) {
        if (rtm->rtm_addrs & (1 << i)) {
          struct sockaddr *sa = reinterpret_cast<struct sockaddr *>(ptr);

          switch (i) {
          case RTAX_DST:
            if (sa->sa_family == AF_INET) {
              struct sockaddr_in *sin =
                  reinterpret_cast<struct sockaddr_in *>(sa);
              inet_ntop(AF_INET, &sin->sin_addr, dst_str, INET_ADDRSTRLEN);
            } else if (sa->sa_family == AF_INET6) {
              struct sockaddr_in6 *sin6 =
                  reinterpret_cast<struct sockaddr_in6 *>(sa);
              inet_ntop(AF_INET6, &sin6->sin6_addr, dst_str, INET6_ADDRSTRLEN);
              scope_id = sin6->sin6_scope_id;
            }
            break;
          case RTAX_GATEWAY:
            if (sa->sa_family == AF_INET) {
              struct sockaddr_in *sin =
                  reinterpret_cast<struct sockaddr_in *>(sa);
              inet_ntop(AF_INET, &sin->sin_addr, gw_str, INET_ADDRSTRLEN);
              gateway_display = std::string(gw_str);
            } else if (sa->sa_family == AF_INET6) {
              struct sockaddr_in6 *sin6 =
                  reinterpret_cast<struct sockaddr_in6 *>(sa);
              inet_ntop(AF_INET6, &sin6->sin6_addr, gw_str, INET6_ADDRSTRLEN);
              gateway_display = std::string(gw_str);
              if (sin6->sin6_scope_id > 0) {
                // Add scope interface for IPv6 link-local addresses
                std::string scope_name = getInterfaceName(sin6->sin6_scope_id);
                if (!scope_name.empty()) {
                  gateway_display += "%" + scope_name;
                }
              }
            } else if (sa->sa_family == AF_LINK) {
              // Gateway is a link-level address
              struct sockaddr_dl *sdl =
                  reinterpret_cast<struct sockaddr_dl *>(sa);
              link_index = sdl->sdl_index;
              if (sdl->sdl_alen > 0) {
                // Convert MAC address to string
                unsigned char *mac = reinterpret_cast<unsigned char *>(
                    sdl->sdl_data + sdl->sdl_nlen);
                snprintf(mac_str, sizeof(mac_str),
                         "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],
                         mac[2], mac[3], mac[4], mac[5]);
                gateway_display = std::string(mac_str);
              } else {
                // Get interface name for link index and display as "ifname
                // (#index)"
                std::string iface_name = getInterfaceName(link_index);
                if (!iface_name.empty()) {
                  gateway_display = iface_name + " (#" +
                                   std::to_string(link_index) + ")";
                } else {
                  gateway_display = "link#" + std::to_string(link_index);
                }
              }
            }
            break;
          case RTAX_NETMASK:
            if (sa->sa_family == AF_INET) {
              struct sockaddr_in *sin =
                  reinterpret_cast<struct sockaddr_in *>(sa);
              inet_ntop(AF_INET, &sin->sin_addr, mask_str, INET_ADDRSTRLEN);
              // Calculate prefix length for IPv4
              uint32_t mask = ntohl(sin->sin_addr.s_addr);
              if (mask == 0) {
                prefix_length = 0; // Default route
              } else {
                prefix_length = 0;
                while (mask & 0x80000000) {
                  prefix_length++;
                  mask <<= 1;
                }
              }
              netmask_display = std::to_string(prefix_length);
            } else if (sa->sa_family == AF_INET6) {
              struct sockaddr_in6 *sin6 =
                  reinterpret_cast<struct sockaddr_in6 *>(sa);
              inet_ntop(AF_INET6, &sin6->sin6_addr, mask_str, INET6_ADDRSTRLEN);
              // Calculate prefix length for IPv6
              prefix_length = 0;
              for (int j = 0; j < 16; j++) {
                uint8_t byte = sin6->sin6_addr.s6_addr[j];
                if (byte == 0xff) {
                  prefix_length += 8;
                } else if (byte != 0) {
                  while (byte & 0x80) {
                    prefix_length++;
                    byte <<= 1;
                  }
                  break;
                }
              }
              netmask_display = std::to_string(prefix_length);
            }
            break;
          case RTAX_IFP:
            if (sa->sa_family == AF_LINK) {
              struct sockaddr_dl *sdl =
                  reinterpret_cast<struct sockaddr_dl *>(sa);
              if (sdl->sdl_nlen > 0) {
                interface = std::string(sdl->sdl_data, sdl->sdl_nlen);
              } else {
                // If no interface name in sockaddr_dl, try to get it from index
                std::string iface_name = getInterfaceName(sdl->sdl_index);
                if (!iface_name.empty()) {
                  interface = iface_name;
                }
              }
              link_index = sdl->sdl_index;
            }
            break;
          case RTAX_BRD:
            // Broadcast address - could be used for additional info
            break;
          case RTAX_GENMASK:
            // Generic mask - could be used for additional info
            break;
          }

          ptr += sa->sa_len;
        }
      }

      // Handle IPv6 scope interface for destination
      std::string destination_display = std::string(dst_str);
      if (scope_id > 0 && std::string(dst_str).find("fe80::") == 0) {
        // Add scope interface for IPv6 link-local destination addresses
        std::string scope_name = getInterfaceName(scope_id);
        if (!scope_name.empty()) {
          destination_display += "%" + scope_name;
        }
      }

      // Get interface from rtm_index like netstat does
      if (interface == "unknown" && rtm->rtm_index > 0) {
        std::string iface_name = getInterfaceName(rtm->rtm_index);
        if (!iface_name.empty()) {
          interface = iface_name;
        }
      }

      // For IPv6 routes, try to get interface from scope_id if interface is
      // still unknown
      if (interface == "unknown" && scope_id > 0) {
        std::string iface_name = getInterfaceName(scope_id);
        if (!iface_name.empty()) {
          interface = iface_name;
        }
      }

      // Handle netmask if not present in routing message
      if (netmask_display.empty()) {
        netmask_display = getNetmaskFromRoutingMessage(rtm);
      }

      // For IPv6 routes, set gateway to interface name (#N) where N is the interface index
      if (gateway_display.empty() && std::string(dst_str).find(':') != std::string::npos && rtm->rtm_index > 0) {
        std::string iface_name = getInterfaceName(rtm->rtm_index);
        if (!iface_name.empty()) {
          gateway_display = iface_name + " (#" + std::to_string(rtm->rtm_index) + ")";
        } else {
          gateway_display = "if (#" + std::to_string(rtm->rtm_index) + ")";
        }
      }

      // Create routing entry info structure
      RoutingEntryInfo info;
      info.destination = destination_display;
      info.gateway =
          gateway_display.empty() ? std::string(gw_str) : gateway_display;
      info.interface = interface;
      info.netmask = netmask_display;
      info.flags = rtm->rtm_flags;

      return std::make_unique<RoutingEntry>(info);
    }
  };

  RoutingTable::RoutingTable() : pImpl(std::make_unique<Impl>()) {}

  RoutingTable::~RoutingTable() = default;

  std::vector<std::unique_ptr<RoutingEntry>> RoutingTable::getEntries() const {
    return pImpl->getEntries();
  }

  std::vector<std::unique_ptr<RoutingEntry>>
  RoutingTable::getEntries(int fib) const {
    return pImpl->getEntries(fib);
  }

  int RoutingTable::getFibCount() const { return pImpl->getFibCount(); }

  int RoutingTable::getDefaultFib() const { return pImpl->getDefaultFib(); }

  std::vector<std::unique_ptr<RoutingEntry>>
  RoutingTable::getEntries(const std::string &destination) const {
    return pImpl->getEntries(destination);
  }

  bool RoutingTable::addEntry(const std::string &destination,
                              const std::string &gateway,
                              const std::string &interface, uint16_t flags) {
    return pImpl->addEntry(destination, gateway, interface, flags);
  }

  bool RoutingTable::addEntry(const std::string &destination,
                              const std::string &gateway,
                              const std::string &interface, uint16_t flags,
                              int fib) {
    return pImpl->addEntry(destination, gateway, interface, flags, fib);
  }

  bool RoutingTable::deleteEntry(const std::string &destination,
                                 const std::string &gateway) {
    return pImpl->deleteEntry(destination, gateway);
  }

  bool RoutingTable::flush() { return pImpl->flush(); }

  std::unique_ptr<RoutingEntry> RoutingTable::getDefaultGateway() const {
    return pImpl->getDefaultGateway();
  }

  bool RoutingTable::isAccessible() const { return pImpl->isAccessible(); }

  std::string RoutingTable::getLastError() const {
    return pImpl->getLastError();
  }

} // namespace libfreebsdnet::routing
