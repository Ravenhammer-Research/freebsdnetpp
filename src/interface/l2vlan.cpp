/**
 * @file interface/l2vlan.cpp
 * @brief L2VLAN interface implementation
 * @details Implementation of L2VLAN interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/l2vlan.hpp>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_mib.h>
#include <net/if_vlan_var.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class L2VlanInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;

    Impl(const std::string &n, unsigned int idx, int f)
        : name(n), index(idx), flags(f) {}
  };

  L2VlanInterface::L2VlanInterface(const std::string &name, unsigned int index,
                                   int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}

  L2VlanInterface::~L2VlanInterface() = default;

  InterfaceType L2VlanInterface::getType() const {
    return InterfaceType::VLAN; // L2VLAN is a type of VLAN
  }

  bool L2VlanInterface::isValid() const {
    return !pImpl->name.empty() && pImpl->index > 0;
  }

  // Base class method implementations
  std::string L2VlanInterface::getName() const { return Interface::getName(); }
  unsigned int L2VlanInterface::getIndex() const {
    return Interface::getIndex();
  }
  std::vector<Flag> L2VlanInterface::getFlags() const {
    return Interface::getFlags();
  }
  bool L2VlanInterface::setFlags(int flags) {
    return Interface::setFlags(flags);
  }
  std::string L2VlanInterface::getLastError() const {
    return Interface::getLastError();
  }

  bool L2VlanInterface::bringUp() { return Interface::bringUp(); }
  bool L2VlanInterface::bringDown() { return Interface::bringDown(); }
  bool L2VlanInterface::isUp() const { return Interface::isUp(); }

  int L2VlanInterface::getMtu() const { return Interface::getMtu(); }
  bool L2VlanInterface::setMtu(int mtu) { return Interface::setMtu(mtu); }

  int L2VlanInterface::getFib() const { return Interface::getFib(); }
  bool L2VlanInterface::setFib(int fib) { return Interface::setFib(fib); }

  int L2VlanInterface::getMedia() const {
    // L2VLAN interfaces typically don't have physical media
    return 0;
  }

  bool L2VlanInterface::setMedia(int media) {
    (void)media; // Suppress unused parameter warning
    pImpl->lastError = "L2VLAN interfaces do not support media setting";
    return false;
  }

  int L2VlanInterface::getMediaStatus() const {
    return 0; // L2VLAN interfaces don't have media status
  }

  int L2VlanInterface::getActiveMedia() const {
    return 0; // L2VLAN interfaces don't have active media
  }

  std::vector<int> L2VlanInterface::getSupportedMedia() const {
    return {}; // L2VLAN interfaces don't have supported media
  }

  uint32_t L2VlanInterface::getCapabilities() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFCAP, &ifr) < 0) {
      close(sock);
      return 0;
    }

    close(sock);
    return ifr.ifr_reqcap;
  }

  bool L2VlanInterface::setCapabilities(uint32_t capabilities) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_reqcap = capabilities;

    if (ioctl(sock, SIOCSIFCAP, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set capabilities: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  uint32_t L2VlanInterface::getEnabledCapabilities() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFCAP, &ifr) < 0) {
      close(sock);
      return 0;
    }

    close(sock);
    return ifr.ifr_curcap;
  }

  bool L2VlanInterface::enableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current | capabilities);
  }

  bool L2VlanInterface::disableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current & ~capabilities);
  }

  std::vector<std::string> L2VlanInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool L2VlanInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool L2VlanInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  bool L2VlanInterface::setPhysicalAddress(const std::string &address) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    // Parse IP address
    struct sockaddr_in *sin =
        reinterpret_cast<struct sockaddr_in *>(&ifra.ifra_addr);
    if (inet_pton(AF_INET, address.c_str(), &sin->sin_addr) != 1) {
      pImpl->lastError = "Invalid IP address format";
      close(sock);
      return false;
    }

    sin->sin_family = AF_INET;
    sin->sin_len = sizeof(*sin);

    if (ioctl(sock, SIOCSIFPHYADDR, &ifra) < 0) {
      pImpl->lastError =
          "Failed to set physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool L2VlanInterface::deletePhysicalAddress() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCDIFPHYADDR, &ifr) < 0) {
      pImpl->lastError =
          "Failed to delete physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool L2VlanInterface::createClone(const std::string &cloneName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, cloneName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
      pImpl->lastError =
          "Failed to create clone: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::vector<std::string> L2VlanInterface::getCloners() const {
    std::vector<std::string> cloners;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return cloners;
    }

    struct if_clonereq ifcr;
    std::memset(&ifcr, 0, sizeof(ifcr));

    // First get the total number of cloners
    if (ioctl(sock, SIOCIFGCLONERS, &ifcr) < 0) {
      close(sock);
      return cloners;
    }

    if (ifcr.ifcr_total > 0) {
      // Allocate buffer for cloner names
      std::vector<char> buffer(ifcr.ifcr_total * IFNAMSIZ);
      ifcr.ifcr_buffer = buffer.data();
      ifcr.ifcr_count = ifcr.ifcr_total;

      // Get the cloner names
      if (ioctl(sock, SIOCIFGCLONERS, &ifcr) == 0) {
        for (int i = 0; i < ifcr.ifcr_count; i++) {
          std::string cloner(buffer.data() + (i * IFNAMSIZ));
          if (!cloner.empty()) {
            cloners.push_back(cloner);
          }
        }
      }
    }

    close(sock);
    return cloners;
  }

  std::string L2VlanInterface::getMacAddress() const {
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

  bool L2VlanInterface::setMacAddress(const std::string &macAddress) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    // Parse MAC address (format: "aa:bb:cc:dd:ee:ff")
    unsigned char mac[6];
    if (std::sscanf(macAddress.c_str(),
                    "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[0],
                    &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
      pImpl->lastError = "Invalid MAC address format";
      close(sock);
      return false;
    }

    // Set the MAC address in the ifreq structure
    std::memcpy(ifr.ifr_addr.sa_data, mac, 6);
    ifr.ifr_addr.sa_family = AF_LINK;
    ifr.ifr_addr.sa_len = 6;

    if (ioctl(sock, SIOCSIFLLADDR, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set MAC address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  // L2VLAN-specific methods
  std::string L2VlanInterface::getTrunkDevice() const {
    // This would require kernel-level access to VLAN_TRUNKDEV macro
    // For now, return empty string
    return "";
  }

  bool L2VlanInterface::setTrunkDevice(const std::string &trunkDevice) {
    (void)trunkDevice; // Suppress unused parameter warning
    pImpl->lastError =
        "Trunk device setting not implemented - requires kernel-level access";
    return false;
  }

  int L2VlanInterface::getVlanTag() const {
    // This would require kernel-level access to VLAN_TAG macro
    // For now, return -1
    return -1;
  }

  bool L2VlanInterface::setVlanTag(int tag) {
    if (tag < 0 || tag > 4095) {
      pImpl->lastError = "Invalid VLAN tag (must be 0-4095)";
      return false;
    }

    // This would require kernel-level access to VLAN_TAG macro
    pImpl->lastError =
        "VLAN tag setting not implemented - requires kernel-level access";
    return false;
  }

  int L2VlanInterface::getPcp() const {
    // This would require kernel-level access to VLAN_PCP macro
    // For now, return -1
    return -1;
  }

  bool L2VlanInterface::setPcp(int pcp) {
    if (pcp < 0 || pcp > 7) {
      pImpl->lastError = "Invalid PCP value (must be 0-7)";
      return false;
    }

    // This would require kernel-level access to VLAN_PCP macro
    pImpl->lastError =
        "PCP setting not implemented - requires kernel-level access";
    return false;
  }

  std::string L2VlanInterface::getVlanCookie() const {
    // This would require kernel-level access to VLAN_COOKIE macro
    // For now, return empty string
    return "";
  }

  bool L2VlanInterface::setVlanCookie(const std::string &cookie) {
    (void)cookie; // Suppress unused parameter warning
    pImpl->lastError =
        "VLAN cookie setting not implemented - requires kernel-level access";
    return false;
  }

  bool L2VlanInterface::destroy() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError =
          "Failed to create socket: " + std::string(strerror(errno));
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
      pImpl->lastError =
          "Failed to destroy interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

} // namespace libfreebsdnet::interface
