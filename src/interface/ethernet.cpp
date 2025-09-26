/**
 * @file interface/ethernet.cpp
 * @brief Ethernet interface implementation
 * @details Implementation of Ethernet interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/ethernet.hpp>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_mib.h>
#include <net/if_private.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <vector>

namespace libfreebsdnet::interface {

  class EthernetInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags) {}
  };

  EthernetInterface::EthernetInterface(const std::string &name,
                                       unsigned int index, int flags)
      : Interface(name, index, flags), pImpl(std::make_unique<Impl>(name, index, flags)) {}

  EthernetInterface::~EthernetInterface() = default;

  // Base class method implementations
  std::string EthernetInterface::getName() const { return Interface::getName(); }
  unsigned int EthernetInterface::getIndex() const { return Interface::getIndex(); }
  InterfaceType EthernetInterface::getType() const { return InterfaceType::ETHERNET; }
  int EthernetInterface::getFlags() const { return Interface::getFlags(); }
  bool EthernetInterface::setFlags(int flags) { return Interface::setFlags(flags); }
  bool EthernetInterface::bringUp() { return Interface::bringUp(); }
  bool EthernetInterface::bringDown() { return Interface::bringDown(); }
  bool EthernetInterface::isUp() const { return Interface::isUp(); }
  int EthernetInterface::getMtu() const { return Interface::getMtu(); }
  bool EthernetInterface::setMtu(int mtu) { return Interface::setMtu(mtu); }
  std::string EthernetInterface::getLastError() const { return Interface::getLastError(); }
  int EthernetInterface::getFib() const { return Interface::getFib(); }
  bool EthernetInterface::setFib(int fib) { return Interface::setFib(fib); }

  std::string EthernetInterface::getMacAddress() const {
    struct ifaddrs *ifaddrs, *ifa;
    std::string macAddress = "";

    if (getifaddrs(&ifaddrs) != 0) {
      return macAddress;
    }

    for (ifa = ifaddrs; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_name && std::string(ifa->ifa_name) == pImpl->name) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
          struct sockaddr_dl *sdl =
              reinterpret_cast<struct sockaddr_dl *>(ifa->ifa_addr);
          if (sdl->sdl_alen == ETHER_ADDR_LEN) {
            unsigned char *addr =
                reinterpret_cast<unsigned char *>(LLADDR(sdl));
            char macStr[18];
            std::snprintf(macStr, sizeof(macStr),
                          "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1],
                          addr[2], addr[3], addr[4], addr[5]);
            macAddress = std::string(macStr);
            break;
          }
        }
      }
    }

    freeifaddrs(ifaddrs);
    return macAddress;
  }

  bool EthernetInterface::setMedia(const std::string &media) {
    (void)media; // Suppress unused parameter warning
    pImpl->lastError =
        "Media setting not implemented - requires specific FreeBSD ioctls";
    return false;
  }

  bool EthernetInterface::supportsPromiscuousMode() const {
    return true; // Most Ethernet interfaces support promiscuous mode
  }

  bool EthernetInterface::enablePromiscuousMode() {
    int newFlags = pImpl->flags | IFF_PROMISC;
    return setFlags(newFlags);
  }

  bool EthernetInterface::disablePromiscuousMode() {
    int newFlags = pImpl->flags & ~IFF_PROMISC;
    return setFlags(newFlags);
  }

  bool EthernetInterface::isPromiscuousModeEnabled() const {
    return (pImpl->flags & IFF_PROMISC) != 0;
  }


  int EthernetInterface::getMedia() const {
    return Interface::getMedia();
  }

  bool EthernetInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int EthernetInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int EthernetInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> EthernetInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t EthernetInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool EthernetInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t EthernetInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool EthernetInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool EthernetInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  std::vector<std::string> EthernetInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool EthernetInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool EthernetInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  int EthernetInterface::getVnet() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return ifr.ifr_jid;
  }

  bool EthernetInterface::setVnet(int vnetId) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_jid = vnetId;

    if (ioctl(sock, SIOCSIFVNET, &ifr) < 0) {
      pImpl->lastError = "Failed to set VNET: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool EthernetInterface::reclaimFromVnet() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCSIFRVNET, &ifr) < 0) {
      pImpl->lastError =
          "Failed to reclaim from VNET: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool EthernetInterface::setPhysicalAddress(const std::string &address) {
    return Interface::setPhysicalAddress(address);
  }

  bool EthernetInterface::deletePhysicalAddress() {
    return Interface::deletePhysicalAddress();
  }

  bool EthernetInterface::createClone(const std::string &cloneName) {
    return Interface::createClone(cloneName);
  }

  std::vector<std::string> EthernetInterface::getCloners() const {
    return Interface::getCloners();
  }

  bool EthernetInterface::setMacAddress(const std::string &macAddress) {
    return Interface::setMacAddress(macAddress);
  }


  bool EthernetInterface::destroy() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket: " + std::string(strerror(errno));
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
      pImpl->lastError = "Failed to destroy interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

} // namespace libfreebsdnet::interface
