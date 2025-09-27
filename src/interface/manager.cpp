/**
 * @file interface/manager.cpp
 * @brief Interface manager implementation
 * @details Implementation of interface management functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <ifaddrs.h>
#include <interface/bridge.hpp>
#include <interface/carp.hpp>
#include <interface/epair.hpp>
#include <interface/ethernet.hpp>
#include <interface/gif.hpp>
#include <interface/l2vlan.hpp>
#include <interface/lagg.hpp>
#include <interface/loopback.hpp>
#include <interface/manager.hpp>
#include <interface/pflog.hpp>
#include <interface/pfsync.hpp>
#include <interface/vlan.hpp>
#include <interface/wireless.hpp>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_lagg.h>
#include <net/if_media.h>
#include <net/if_types.h>
#include <net80211/ieee80211_ioctl.h>
#include <netinet/in.h>
#include <set>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  Manager::Manager() {
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
      throw std::runtime_error(
          "Failed to create socket for interface operations");
    }
  }

  Manager::~Manager() {
    if (socket_fd >= 0) {
      close(socket_fd);
    }
  }

  std::vector<std::unique_ptr<Interface>> Manager::getInterfaces() const {
    std::vector<std::unique_ptr<Interface>> interfaces;
    struct ifaddrs *ifaddrs_ptr;

    if (getifaddrs(&ifaddrs_ptr) == -1) {
      return interfaces;
    }

    for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr;
         ifa = ifa->ifa_next) {
      // Only process link-level addresses (AF_LINK) to avoid duplicates
      if (ifa->ifa_name && ifa->ifa_addr &&
          ifa->ifa_addr->sa_family == AF_LINK) {
        std::string if_name(ifa->ifa_name);
        unsigned int index = if_nametoindex(ifa->ifa_name);

        struct ifreq ifr;
        std::strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';

        if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) == 0) {
          // Create the correct interface class based on name pattern matching
          std::unique_ptr<Interface> interface = nullptr;
          std::string name(ifa->ifa_name);

          if (name.substr(0, 6) == "bridge") {
            interface = std::make_unique<BridgeInterface>(ifa->ifa_name, index,
                                                          ifr.ifr_flags);
          } else if (name.substr(0, 4) == "lagg") {
            interface = std::make_unique<LagInterface>(ifa->ifa_name, index,
                                                       ifr.ifr_flags);
          } else if (name.substr(0, 3) == "gif") {
            interface = std::make_unique<GifInterface>(ifa->ifa_name, index,
                                                       ifr.ifr_flags);
          } else if (name.substr(0, 2) == "lo") {
            interface = std::make_unique<LoopbackInterface>(
                ifa->ifa_name, index, ifr.ifr_flags);
          } else if (name.substr(0, 5) == "epair") {
            interface = std::make_unique<EpairInterface>(ifa->ifa_name, index,
                                                         ifr.ifr_flags);
          } else if (name.substr(0, 4) == "vlan") {
            interface = std::make_unique<VlanInterface>(ifa->ifa_name, index,
                                                        ifr.ifr_flags);
          } else if (name.substr(0, 6) == "l2vlan") {
            interface = std::make_unique<L2VlanInterface>(ifa->ifa_name, index,
                                                          ifr.ifr_flags);
          } else if (name.substr(0, 5) == "pfsync") {
            interface = std::make_unique<PfsyncInterface>(ifa->ifa_name, index,
                                                          ifr.ifr_flags);
          } else if (name.substr(0, 5) == "pflog") {
            interface = std::make_unique<PflogInterface>(ifa->ifa_name, index,
                                                         ifr.ifr_flags);
          } else if (name.substr(0, 4) == "carp") {
            interface = std::make_unique<CarpInterface>(ifa->ifa_name, index,
                                                        ifr.ifr_flags);
          } else {
            // Default to Ethernet for everything else
            interface = std::make_unique<EthernetInterface>(
                ifa->ifa_name, index, ifr.ifr_flags);
          }

          if (interface) {
            interfaces.push_back(std::move(interface));
          }
        }
      }
    }

    freeifaddrs(ifaddrs_ptr);
    return interfaces;
  }

  std::unique_ptr<Interface>
  Manager::getInterface(const std::string &name) const {
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
          // Get the actual interface type from the system using sockaddr_dl
          std::unique_ptr<Interface> interface = nullptr;

          // Check if this is a link-level address (sockaddr_dl)
          if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
            // Create the correct interface class based on name pattern matching
            std::string if_name(ifa->ifa_name);

            // First check if this is a bridge or lagg by name pattern
            if (if_name.substr(0, 6) == "bridge") {
              interface = std::make_unique<BridgeInterface>(
                  ifa->ifa_name, index, ifr.ifr_flags);
            } else if (if_name.substr(0, 4) == "lagg") {
              interface = std::make_unique<LagInterface>(ifa->ifa_name, index,
                                                         ifr.ifr_flags);
            } else if (if_name.substr(0, 3) == "gif") {
              interface = std::make_unique<GifInterface>(ifa->ifa_name, index,
                                                         ifr.ifr_flags);
            } else if (if_name.substr(0, 2) == "lo") {
              interface = std::make_unique<LoopbackInterface>(
                  ifa->ifa_name, index, ifr.ifr_flags);
            } else if (if_name.substr(0, 5) == "epair") {
              interface = std::make_unique<EpairInterface>(ifa->ifa_name, index,
                                                           ifr.ifr_flags);
            } else if (if_name.substr(0, 4) == "vlan") {
              interface = std::make_unique<VlanInterface>(ifa->ifa_name, index,
                                                          ifr.ifr_flags);
            } else if (if_name.substr(0, 6) == "l2vlan") {
              interface = std::make_unique<L2VlanInterface>(
                  ifa->ifa_name, index, ifr.ifr_flags);
            } else if (if_name.substr(0, 5) == "pfsync") {
              interface = std::make_unique<PfsyncInterface>(
                  ifa->ifa_name, index, ifr.ifr_flags);
            } else if (if_name.substr(0, 5) == "pflog") {
              interface = std::make_unique<PflogInterface>(ifa->ifa_name, index,
                                                           ifr.ifr_flags);
            } else if (if_name.substr(0, 4) == "carp") {
              interface = std::make_unique<CarpInterface>(ifa->ifa_name, index,
                                                          ifr.ifr_flags);
            } else {
              // Default to Ethernet for everything else
              interface = std::make_unique<EthernetInterface>(
                  ifa->ifa_name, index, ifr.ifr_flags);
            }
          }

          // Fallback to EthernetInterface if type detection failed
          if (!interface) {
            interface = std::make_unique<EthernetInterface>(
                ifa->ifa_name, index, ifr.ifr_flags);
          }

          freeifaddrs(ifaddrs_ptr);
          return interface;
        }
      }
    }

    freeifaddrs(ifaddrs_ptr);
    return nullptr;
  }

  std::unique_ptr<Interface> Manager::getInterface(unsigned int index) const {
    struct ifaddrs *ifaddrs_ptr;
    if (getifaddrs(&ifaddrs_ptr) == -1) {
      return nullptr;
    }

    for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr;
         ifa = ifa->ifa_next) {
      if (ifa->ifa_name && ifa->ifa_addr) {
        unsigned int if_index = if_nametoindex(ifa->ifa_name);
        if (if_index == index) {
          auto interface = getInterface(ifa->ifa_name);
          freeifaddrs(ifaddrs_ptr);
          return interface;
        }
      }
    }

    freeifaddrs(ifaddrs_ptr);
    return nullptr;
  }

  bool Manager::interfaceExists(const std::string &name) const {
    return getInterface(name) != nullptr;
  }

  int Manager::getInterfaceFlags(const std::string &name) const {
    struct ifreq ifr;
    std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) == 0) {
      return ifr.ifr_flags;
    }
    return 0;
  }

  bool Manager::setInterfaceFlags(const std::string &name, int flags) {
    struct ifreq ifr;
    std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    ifr.ifr_flags = flags;

    return ioctl(socket_fd, SIOCSIFFLAGS, &ifr) == 0;
  }

  bool Manager::bringUp(const std::string &name) {
    int flags = getInterfaceFlags(name);
    return setInterfaceFlags(name, flags | IFF_UP);
  }

  bool Manager::bringDown(const std::string &name) {
    int flags = getInterfaceFlags(name);
    return setInterfaceFlags(name, flags & ~IFF_UP);
  }

  std::unique_ptr<Interface> Manager::createInterface(const std::string &name,
                                                      unsigned int index,
                                                      int flags) {
    // Get the actual interface type from the system using sockaddr_dl
    struct ifaddrs *ifaddrs_ptr;
    std::unique_ptr<Interface> interface = nullptr;

    if (getifaddrs(&ifaddrs_ptr) == 0) {
      for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr;
           ifa = ifa->ifa_next) {
        if (ifa->ifa_name && std::string(ifa->ifa_name) == name) {
          // Check if this is a link-level address (sockaddr_dl)
          if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl *sdl =
                reinterpret_cast<struct sockaddr_dl *>(ifa->ifa_addr);

            // Create the correct interface class based on sdl_type and ioctl
            // capabilities Some interfaces (bridge, lagg) present as Ethernet
            // at link layer but need to be detected by their specific ioctl
            // support
            std::string if_name(ifa->ifa_name);

            // Test for LAGG interface using SIOCSLAGG ioctl
            struct lagg_reqall lagg_req;
            std::memset(&lagg_req, 0, sizeof(lagg_req));
            std::strncpy(lagg_req.ra_ifname, ifa->ifa_name, IFNAMSIZ - 1);
            lagg_req.ra_ifname[IFNAMSIZ - 1] = '\0';

            if (ioctl(socket_fd, SIOCSLAGG, &lagg_req) == 0 ||
                errno == EINVAL) {
              // Interface supports LAGG ioctl
              interface = std::make_unique<LagInterface>(name, index, flags);
            } else {
              // Test for Bridge interface using SIOCGDRVSPEC ioctl
              struct ifdrv ifd;
              std::memset(&ifd, 0, sizeof(ifd));
              std::strncpy(ifd.ifd_name, ifa->ifa_name, IFNAMSIZ - 1);
              ifd.ifd_name[IFNAMSIZ - 1] = '\0';
              ifd.ifd_cmd = 0; // Test command

              if (ioctl(socket_fd, SIOCGDRVSPEC, &ifd) == 0 ||
                  errno == EINVAL) {
                // Interface supports bridge ioctl
                interface =
                    std::make_unique<BridgeInterface>(name, index, flags);
              } else {
                // Use sdl_type for other interfaces
                switch (sdl->sdl_type) {
                case IFT_ETHER:
                  interface =
                      std::make_unique<EthernetInterface>(name, index, flags);
                  break;
                case IFT_LOOP:
                  interface =
                      std::make_unique<LoopbackInterface>(name, index, flags);
                  break;
                case IFT_BRIDGE:
                  interface =
                      std::make_unique<BridgeInterface>(name, index, flags);
                  break;
                case IFT_IEEE80211:
                  interface =
                      std::make_unique<WirelessInterface>(name, index, flags);
                  break;
                case IFT_L2VLAN:
                  interface =
                      std::make_unique<L2VlanInterface>(name, index, flags);
                  break;
                case 0x88: // IFT_L3IPVLAN (EPAIR)
                  interface =
                      std::make_unique<EpairInterface>(name, index, flags);
                  break;
                case 0x89: // IFT_L3IPXVLAN (LAGG)
                  interface =
                      std::make_unique<LagInterface>(name, index, flags);
                  break;
                default:
                  interface =
                      std::make_unique<EthernetInterface>(name, index, flags);
                  break;
                }
              }
            }
            break;
          }
        }
      }
      freeifaddrs(ifaddrs_ptr);
    }

    // Fallback to EthernetInterface if type detection failed
    if (!interface) {
      interface = std::make_unique<EthernetInterface>(name, index, flags);
    }

    return interface;
  }

  std::unique_ptr<Interface> Manager::createInterface(const std::string &name,
                                                      unsigned int index,
                                                      int flags,
                                                      InterfaceType type) {
    switch (type) {
    case InterfaceType::ETHERNET:
      return std::make_unique<EthernetInterface>(name, index, flags);
    case InterfaceType::BRIDGE:
      return std::make_unique<BridgeInterface>(name, index, flags);
    case InterfaceType::VLAN:
      return std::make_unique<VlanInterface>(name, index, flags);
    case InterfaceType::LAGG:
      return std::make_unique<LagInterface>(name, index, flags);
    case InterfaceType::PFSYNC:
      return std::make_unique<PfsyncInterface>(name, index, flags);
    case InterfaceType::PFLOG:
      return std::make_unique<PflogInterface>(name, index, flags);
    case InterfaceType::CARP:
      return std::make_unique<CarpInterface>(name, index, flags);
    case InterfaceType::L2VLAN:
      return std::make_unique<L2VlanInterface>(name, index, flags);
    case InterfaceType::WIRELESS:
      return std::make_unique<WirelessInterface>(name, index, flags);
    case InterfaceType::EPAIR:
      return std::make_unique<EpairInterface>(name, index, flags);
    case InterfaceType::LOOPBACK:
      return std::make_unique<LoopbackInterface>(name, index, flags);
    case InterfaceType::GIF:
      return std::make_unique<GifInterface>(name, index, flags);
    default:
      return nullptr;
    }
  }

  bool Manager::isSupported(InterfaceType type) {
    switch (type) {
    case InterfaceType::ETHERNET:
    case InterfaceType::BRIDGE:
    case InterfaceType::VLAN:
    case InterfaceType::LAGG:
    case InterfaceType::GIF:
    case InterfaceType::PFSYNC:
    case InterfaceType::PFLOG:
    case InterfaceType::CARP:
    case InterfaceType::L2VLAN:
    case InterfaceType::WIRELESS:
    case InterfaceType::EPAIR:
    case InterfaceType::LOOPBACK:
      return true;
    default:
      return false;
    }
  }

  std::vector<InterfaceType> Manager::getSupportedTypes() {
    return {InterfaceType::ETHERNET, InterfaceType::BRIDGE,
            InterfaceType::VLAN,     InterfaceType::LAGG,
            InterfaceType::GIF,      InterfaceType::PFSYNC,
            InterfaceType::PFLOG,    InterfaceType::CARP,
            InterfaceType::L2VLAN,   InterfaceType::WIRELESS,
            InterfaceType::EPAIR,    InterfaceType::LOOPBACK};
  }

  InterfaceType Manager::getTypeFromFlags(int flags) {
    if (flags & IFF_LOOPBACK) {
      return InterfaceType::LOOPBACK;
    }
    return InterfaceType::ETHERNET;
  }

} // namespace libfreebsdnet::interface