/**
 * @file interface/manager.cpp
 * @brief Network interface manager implementation
 * @details Provides C++ wrapper for FreeBSD network interface management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>
#include <interface/manager.hpp>
#include <net/if.h>
#include <netinet/in.h>
#include <set>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  // InterfaceInfo implementation
  InterfaceInfo::InterfaceInfo(const std::string &name, unsigned int index,
                               int flags)
      : name(name), index(index), flags(flags), type(0), mtu(0) {}

  // Manager implementation
  class Manager::Impl {
  private:
    int socket_fd;

  public:
    Impl() : socket_fd(-1) {
      socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
      if (socket_fd < 0) {
        throw std::runtime_error(
            "Failed to create socket for interface operations");
      }
    }

    ~Impl() {
      if (socket_fd >= 0) {
        close(socket_fd);
      }
    }

    std::vector<InterfaceInfo> getInterfaces() const {
      std::vector<InterfaceInfo> interfaces;
      std::set<std::string> seen_interfaces;

      struct ifaddrs *ifaddrs_ptr;
      if (getifaddrs(&ifaddrs_ptr) == -1) {
        throw std::runtime_error("Failed to get interface addresses");
      }

      for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr;
           ifa = ifa->ifa_next) {
        if (ifa->ifa_name && ifa->ifa_addr) {
          std::string name = ifa->ifa_name;

          // Skip if we've already seen this interface
          if (seen_interfaces.find(name) != seen_interfaces.end()) {
            continue;
          }

          seen_interfaces.insert(name);

          // Get interface index
          unsigned int index = if_nametoindex(ifa->ifa_name);

          // Get interface flags
          struct ifreq ifr;
          std::strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
          ifr.ifr_name[IFNAMSIZ - 1] = '\0';

          if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) == 0) {
            InterfaceInfo info(ifa->ifa_name, index, ifr.ifr_flags);

             // Get interface type and MTU
             // For now, we'll use a simple approach based on interface name
             // This is a temporary solution until we implement proper interface
             // type detection
             std::string if_name = ifa->ifa_name;
             if (if_name.substr(0, 2) == "lo") {
               info.type = 0x18; // IFT_LOOP
             } else if (if_name.substr(0, 5) == "epair") {
               info.type = 0x88; // IFT_EPAIR (custom type for epair)
             } else if (if_name.substr(0, 3) == "eth" ||
                        if_name.substr(0, 2) == "em" ||
                        if_name.substr(0, 3) == "igb" ||
                        if_name.substr(0, 3) == "ixg" ||
                        if_name.substr(0, 3) == "bge" ||
                        if_name.substr(0, 3) == "fxp" ||
                        if_name.substr(0, 2) == "re") {
               info.type = 0x06; // IFT_ETHER
             } else if (if_name.substr(0, 6) == "bridge") {
               info.type = 0xd1; // IFT_BRIDGE
             } else if (if_name.substr(0, 4) == "wlan") {
               info.type = 0x71; // IFT_IEEE80211
             } else if (if_name.find('.') != std::string::npos) {
               info.type = 0x87; // IFT_L2VLAN
             }

            // Try to get MTU using SIOCGIFMTU
            struct ifreq ifr_mtu_req;
            std::strncpy(ifr_mtu_req.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
            ifr_mtu_req.ifr_name[IFNAMSIZ - 1] = '\0';
            if (ioctl(socket_fd, SIOCGIFMTU, &ifr_mtu_req) == 0) {
              info.mtu = ifr_mtu_req.ifr_mtu;
            }

            // Get IP addresses
            for (struct ifaddrs *addr_ifa = ifaddrs_ptr; addr_ifa != nullptr;
                 addr_ifa = addr_ifa->ifa_next) {
              if (addr_ifa->ifa_name &&
                  std::string(addr_ifa->ifa_name) == name &&
                  addr_ifa->ifa_addr) {
                if (addr_ifa->ifa_addr->sa_family == AF_INET) {
                  struct sockaddr_in *addr_in =
                      (struct sockaddr_in *)addr_ifa->ifa_addr;
                  char ip_str[INET_ADDRSTRLEN];
                  if (inet_ntop(AF_INET, &addr_in->sin_addr, ip_str,
                                INET_ADDRSTRLEN)) {
                    info.addresses.push_back(ip_str);
                  }
                } else if (addr_ifa->ifa_addr->sa_family == AF_INET6) {
                  struct sockaddr_in6 *addr_in6 =
                      (struct sockaddr_in6 *)addr_ifa->ifa_addr;
                  char ip_str[INET6_ADDRSTRLEN];
                  if (inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str,
                                INET6_ADDRSTRLEN)) {
                    info.addresses.push_back(ip_str);
                  }
                }
              }
            }

            interfaces.push_back(info);
          }
        }
      }

      freeifaddrs(ifaddrs_ptr);
      return interfaces;
    }

    std::unique_ptr<InterfaceInfo> getInterface(const std::string &name) const {
      struct ifaddrs *ifaddrs_ptr;
      if (getifaddrs(&ifaddrs_ptr) == -1) {
        return nullptr;
      }

      for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr;
           ifa = ifa->ifa_next) {
        if (ifa->ifa_name && std::string(ifa->ifa_name) == name &&
            ifa->ifa_addr) {
          unsigned int index = if_nametoindex(ifa->ifa_name);

          struct ifreq ifr;
          std::strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
          ifr.ifr_name[IFNAMSIZ - 1] = '\0';

          if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) == 0) {
            auto info = std::make_unique<InterfaceInfo>(ifa->ifa_name, index,
                                                        ifr.ifr_flags);

             // Get interface type and MTU
             std::string if_name = ifa->ifa_name;
             if (if_name.substr(0, 2) == "lo") {
               info->type = 0x18; // IFT_LOOP
             } else if (if_name.substr(0, 5) == "epair") {
               info->type = 0x88; // IFT_EPAIR (custom type for epair)
             } else if (if_name.substr(0, 3) == "eth" ||
                        if_name.substr(0, 2) == "em" ||
                        if_name.substr(0, 3) == "igb" ||
                        if_name.substr(0, 3) == "ixg" ||
                        if_name.substr(0, 3) == "bge" ||
                        if_name.substr(0, 3) == "fxp" ||
                        if_name.substr(0, 2) == "re") {
               info->type = 0x06; // IFT_ETHER
             } else if (if_name.substr(0, 6) == "bridge") {
               info->type = 0xd1; // IFT_BRIDGE
             } else if (if_name.substr(0, 4) == "wlan") {
               info->type = 0x71; // IFT_IEEE80211
             } else if (if_name.find('.') != std::string::npos) {
               info->type = 0x87; // IFT_L2VLAN
             }

            // Try to get MTU using SIOCGIFMTU
            struct ifreq ifr_mtu_req;
            std::strncpy(ifr_mtu_req.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
            ifr_mtu_req.ifr_name[IFNAMSIZ - 1] = '\0';
            if (ioctl(socket_fd, SIOCGIFMTU, &ifr_mtu_req) == 0) {
              info->mtu = ifr_mtu_req.ifr_mtu;
            }

            // Get IP addresses
            for (struct ifaddrs *addr_ifa = ifaddrs_ptr; addr_ifa != nullptr;
                 addr_ifa = addr_ifa->ifa_next) {
              if (addr_ifa->ifa_name &&
                  std::string(addr_ifa->ifa_name) == name &&
                  addr_ifa->ifa_addr) {
                if (addr_ifa->ifa_addr->sa_family == AF_INET) {
                  struct sockaddr_in *addr_in =
                      (struct sockaddr_in *)addr_ifa->ifa_addr;
                  char ip_str[INET_ADDRSTRLEN];
                  if (inet_ntop(AF_INET, &addr_in->sin_addr, ip_str,
                                INET_ADDRSTRLEN)) {
                    info->addresses.push_back(ip_str);
                  }
                } else if (addr_ifa->ifa_addr->sa_family == AF_INET6) {
                  struct sockaddr_in6 *addr_in6 =
                      (struct sockaddr_in6 *)addr_ifa->ifa_addr;
                  char ip_str[INET6_ADDRSTRLEN];
                  if (inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str,
                                INET6_ADDRSTRLEN)) {
                    info->addresses.push_back(ip_str);
                  }
                }
              }
            }

            freeifaddrs(ifaddrs_ptr);
            return info;
          }
        }
      }

      freeifaddrs(ifaddrs_ptr);
      return nullptr;
    }

    std::unique_ptr<InterfaceInfo> getInterface(unsigned int index) const {
      struct ifaddrs *ifaddrs_ptr;
      if (getifaddrs(&ifaddrs_ptr) == -1) {
        return nullptr;
      }

      for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr;
           ifa = ifa->ifa_next) {
        if (ifa->ifa_name && ifa->ifa_addr) {
          unsigned int if_index = if_nametoindex(ifa->ifa_name);
          if (if_index == index) {
            auto info = getInterface(ifa->ifa_name);
            freeifaddrs(ifaddrs_ptr);
            return info;
          }
        }
      }

      freeifaddrs(ifaddrs_ptr);
      return nullptr;
    }

    bool interfaceExists(const std::string &name) const {
      return getInterface(name) != nullptr;
    }

    int getInterfaceFlags(const std::string &name) const {
      struct ifreq ifr;
      std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
      ifr.ifr_name[IFNAMSIZ - 1] = '\0';

      if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) != 0) {
        return -1;
      }

      return ifr.ifr_flags;
    }

    bool setInterfaceFlags(const std::string &name, int flags) {
      struct ifreq ifr;
      std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
      ifr.ifr_name[IFNAMSIZ - 1] = '\0';

      // Get current flags first
      if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) != 0) {
        return false;
      }

      // Set new flags
      ifr.ifr_flags = flags;
      return ioctl(socket_fd, SIOCSIFFLAGS, &ifr) == 0;
    }

    bool bringUp(const std::string &name) {
      int flags = getInterfaceFlags(name);
      if (flags == -1) {
        return false;
      }

      return setInterfaceFlags(name, flags | IFF_UP);
    }

    bool bringDown(const std::string &name) {
      int flags = getInterfaceFlags(name);
      if (flags == -1) {
        return false;
      }

      return setInterfaceFlags(name, flags & ~IFF_UP);
    }
  };

  Manager::Manager() : pImpl(std::make_unique<Impl>()) {}

  Manager::~Manager() = default;

  std::vector<InterfaceInfo> Manager::getInterfaces() const {
    return pImpl->getInterfaces();
  }

  std::unique_ptr<InterfaceInfo>
  Manager::getInterface(const std::string &name) const {
    return pImpl->getInterface(name);
  }

  std::unique_ptr<InterfaceInfo>
  Manager::getInterface(unsigned int index) const {
    return pImpl->getInterface(index);
  }

  bool Manager::interfaceExists(const std::string &name) const {
    return pImpl->interfaceExists(name);
  }

  int Manager::getInterfaceFlags(const std::string &name) const {
    return pImpl->getInterfaceFlags(name);
  }

  bool Manager::setInterfaceFlags(const std::string &name, int flags) {
    return pImpl->setInterfaceFlags(name, flags);
  }

  bool Manager::bringUp(const std::string &name) {
    return pImpl->bringUp(name);
  }

  bool Manager::bringDown(const std::string &name) {
    return pImpl->bringDown(name);
  }

  InterfaceType Manager::getInterfaceTypeFromNumeric(int type) {
    switch (type) {
    case 0x06:
      return InterfaceType::ETHERNET; // IFT_ETHER
    case 0x18:
      return InterfaceType::LOOPBACK; // IFT_LOOP
    case 0xd1:
      return InterfaceType::BRIDGE; // IFT_BRIDGE
    case 0x71:
      return InterfaceType::WIRELESS; // IFT_IEEE80211
    case 0x87:
      return InterfaceType::L2VLAN; // IFT_L2VLAN
    case 0x88:
      return InterfaceType::EPAIR; // IFT_EPAIR (assuming this is the type)
    default:
      return InterfaceType::ETHERNET; // Default fallback
    }
  }

} // namespace libfreebsdnet::interface