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

    std::vector<std::unique_ptr<RoutingEntry>> getEntries() const {
      return getEntries(0); // Default FIB
    }

    std::vector<std::unique_ptr<RoutingEntry>> getEntries(int fib) const {
      std::vector<std::unique_ptr<RoutingEntry>> entries;

      // Use sysctl to get routing table for specific FIB (like FreeBSD
      // netstat/route)
      size_t len = 0;
      int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_DUMP, 0, fib};

      // Get the size first (like FreeBSD netstat does)
      if (sysctl(mib, 7, nullptr, &len, nullptr, 0) < 0) {
        lastError_ =
            "Failed to get routing table size: " + std::string(strerror(errno));
        return entries;
      }

      if (len == 0) {
        return entries;
      }

      // Allocate buffer
      std::vector<char> buffer(len);

      // Get the routing table (like FreeBSD netstat does)
      if (sysctl(mib, 7, buffer.data(), &len, nullptr, 0) < 0) {
        lastError_ =
            "Failed to get routing table: " + std::string(strerror(errno));
        return entries;
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

    std::unique_ptr<RoutingEntry>
    parseRoutingMessage(struct rt_msghdr *rtm) const {
      // Complete routing message parser handling all address types

      char dst_str[INET_ADDRSTRLEN] = {0};
      char gw_str[INET_ADDRSTRLEN] = {0};
      char mask_str[INET_ADDRSTRLEN] = {0};
      char mac_str[18] = {0}; // MAC address string buffer
      std::string interface = "unknown";

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
              inet_ntop(AF_INET6, &sin6->sin6_addr, dst_str, INET_ADDRSTRLEN);
            }
            break;
          case RTAX_GATEWAY:
            if (sa->sa_family == AF_INET) {
              struct sockaddr_in *sin =
                  reinterpret_cast<struct sockaddr_in *>(sa);
              inet_ntop(AF_INET, &sin->sin_addr, gw_str, INET_ADDRSTRLEN);
            } else if (sa->sa_family == AF_INET6) {
              struct sockaddr_in6 *sin6 =
                  reinterpret_cast<struct sockaddr_in6 *>(sa);
              inet_ntop(AF_INET6, &sin6->sin6_addr, gw_str, INET_ADDRSTRLEN);
            } else if (sa->sa_family == AF_LINK) {
              // Gateway is a link-level address
              struct sockaddr_dl *sdl =
                  reinterpret_cast<struct sockaddr_dl *>(sa);
              if (sdl->sdl_alen > 0) {
                // Convert MAC address to string
                unsigned char *mac = reinterpret_cast<unsigned char *>(
                    sdl->sdl_data + sdl->sdl_nlen);
                snprintf(mac_str, sizeof(mac_str),
                         "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],
                         mac[2], mac[3], mac[4], mac[5]);
                std::strncpy(gw_str, mac_str, sizeof(gw_str) - 1);
                gw_str[sizeof(gw_str) - 1] = '\0';
              }
            }
            break;
          case RTAX_NETMASK:
            if (sa->sa_family == AF_INET) {
              struct sockaddr_in *sin =
                  reinterpret_cast<struct sockaddr_in *>(sa);
              inet_ntop(AF_INET, &sin->sin_addr, mask_str, INET_ADDRSTRLEN);
            } else if (sa->sa_family == AF_INET6) {
              struct sockaddr_in6 *sin6 =
                  reinterpret_cast<struct sockaddr_in6 *>(sa);
              inet_ntop(AF_INET6, &sin6->sin6_addr, mask_str, INET_ADDRSTRLEN);
            }
            break;
          case RTAX_IFP:
            if (sa->sa_family == AF_LINK) {
              struct sockaddr_dl *sdl =
                  reinterpret_cast<struct sockaddr_dl *>(sa);
              if (sdl->sdl_nlen > 0) {
                interface = std::string(sdl->sdl_data, sdl->sdl_nlen);
              }
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

      // Create routing entry info structure
      RoutingEntryInfo info;
      info.destination = std::string(dst_str);
      info.gateway = std::string(gw_str);
      info.interface = interface;
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
