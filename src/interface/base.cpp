/**
 * @file interface/base.cpp
 * @brief Base interface class implementation
 * @details Implementation of base interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/base.hpp>
#include <interface/manager.hpp>
#include <memory>
#include <net/if.h>
#include <net/if_media.h>
#include <net80211/ieee80211_ioctl.h>
#include <string>
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  std::unique_ptr<Interface> createInterface(const std::string &name,
                                             unsigned int index, int flags) {
    Manager manager;
    return manager.createInterface(name, index, flags);
  }

  // Default implementation for getAddresses that can be used by all interfaces
  std::vector<libfreebsdnet::types::Address> Interface::getAddresses() const {
    std::vector<libfreebsdnet::types::Address> addresses;
    struct ifaddrs *ifaddrs_ptr;
    
    if (getifaddrs(&ifaddrs_ptr) == -1) {
      return addresses;
    }
    
    for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_name && ifa->ifa_addr && 
          std::string(ifa->ifa_name) == getName()) {
        if (ifa->ifa_addr->sa_family == AF_INET) {
          struct sockaddr_in *addr_in = (struct sockaddr_in *)ifa->ifa_addr;
          char ip_str[INET_ADDRSTRLEN];
          if (inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, INET_ADDRSTRLEN)) {
            // For now, we'll use a default prefix length of 24 for IPv4
            // In a real implementation, you'd want to get the actual prefix length
            std::string addressStr = std::string(ip_str) + "/24";
            addresses.emplace_back(addressStr);
          }
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
          struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)ifa->ifa_addr;
          char ip_str[INET6_ADDRSTRLEN];
          if (inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str, INET6_ADDRSTRLEN)) {
            // For IPv6, use a default prefix length of 64
            std::string addressStr = std::string(ip_str) + "/64";
            addresses.emplace_back(addressStr);
          }
        }
      }
    }
    
    freeifaddrs(ifaddrs_ptr);
    return addresses;
  }

  // Default implementation for setAddress that can be used by all interfaces
  bool Interface::setAddress(const libfreebsdnet::types::Address &address) {
    if (!address.isValid()) {
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    // Get sockaddr from Address object
    struct sockaddr_in addr = address.getSockaddrIn();
    if (addr.sin_family != AF_INET) {
      close(sock);
      return false;
    }

    // Calculate netmask from prefix length
    uint32_t netmask = 0xFFFFFFFF << (32 - address.getPrefixLength());
    struct sockaddr_in mask;
    mask.sin_family = AF_INET;
    mask.sin_addr.s_addr = htonl(netmask);

    // Calculate broadcast address
    struct sockaddr_in broadcast;
    broadcast.sin_family = AF_INET;
    broadcast.sin_addr.s_addr = addr.sin_addr.s_addr | ~netmask;

    // Set up ifaliasreq
    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, getName().c_str(), IFNAMSIZ - 1);
    std::memcpy(&ifra.ifra_addr, &addr, sizeof(addr));
    std::memcpy(&ifra.ifra_mask, &mask, sizeof(mask));
    std::memcpy(&ifra.ifra_broadaddr, &broadcast, sizeof(broadcast));

    // Add the address
    bool result = (ioctl(sock, SIOCAIFADDR, &ifra) == 0);
    close(sock);
    return result;
  }

  // String overload for setAddress
  bool Interface::setAddress(const std::string &addressString) {
    libfreebsdnet::types::Address address(addressString);
    return setAddress(address);
  }

  // Common implementations for all interfaces
  bool Interface::setFlags(int flags) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);
    ifr.ifr_flags = flags;

    bool result = (ioctl(sock, SIOCSIFFLAGS, &ifr) == 0);
    close(sock);
    return result;
  }

  bool Interface::bringUp() {
    int currentFlags = getFlags();
    int newFlags = currentFlags | IFF_UP;
    return setFlags(newFlags);
  }

  bool Interface::bringDown() {
    int currentFlags = getFlags();
    int newFlags = currentFlags & ~IFF_UP;
    return setFlags(newFlags);
  }

  bool Interface::setMtu(int mtu) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);
    ifr.ifr_mtu = mtu;

    bool result = (ioctl(sock, SIOCSIFMTU, &ifr) == 0);
    close(sock);
    return result;
  }

  int Interface::getFib() const {
    // Get FIB assignment using the correct FreeBSD ioctl (like ifconfig does)
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0; // Default FIB
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFFIB, &ifr) < 0) {
      close(sock);
      return 0; // Default FIB on error
    }

    close(sock);
    return ifr.ifr_fib;
  }

  bool Interface::setFib(int fib) {
    // XXX Set FIB assignment using the correct FreeBSD ioctl (like ifconfig does)
    // Try AF_INET first, fall back to AF_LOCAL if that fails
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0 && errno == EAFNOSUPPORT) {
      sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    }
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);
    ifr.ifr_fib = fib;

    bool result = (ioctl(sock, SIOCSIFFIB, &ifr) == 0);
    close(sock);
    return result;
  }

  std::string Interface::getName() const {
    return pImpl ? pImpl->name : "";
  }

  unsigned int Interface::getIndex() const {
    return pImpl ? pImpl->index : 0;
  }

  int Interface::getFlags() const {
    return pImpl ? pImpl->flags : 0;
  }

  bool Interface::isUp() const {
    return pImpl ? (pImpl->flags & IFF_UP) != 0 : false;
  }

  int Interface::getMtu() const {
    if (!pImpl) return 1500;
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return 1500;
    
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    
    int mtu = 1500;
    if (ioctl(sock, SIOCGIFMTU, &ifr) == 0) {
      mtu = ifr.ifr_mtu;
    }
    
    close(sock);
    return mtu;
  }

  InterfaceType Interface::getType() const {
    throw std::runtime_error("getType() must be overridden by derived classes");
  }

  std::string Interface::getLastError() const {
    return ""; // Default implementation returns empty string
  }

  bool Interface::setAliasAddress(const libfreebsdnet::types::Address &address) {
    if (!address.isValid()) {
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    // Get sockaddr from Address object
    struct sockaddr_in addr = address.getSockaddrIn();
    if (addr.sin_family != AF_INET) {
      close(sock);
      return false;
    }

    // Calculate netmask from prefix length
    uint32_t netmask = 0xFFFFFFFF << (32 - address.getPrefixLength());
    struct sockaddr_in mask;
    mask.sin_family = AF_INET;
    mask.sin_addr.s_addr = htonl(netmask);

    // Calculate broadcast address
    struct sockaddr_in broadcast;
    broadcast.sin_family = AF_INET;
    broadcast.sin_addr.s_addr = addr.sin_addr.s_addr | ~netmask;

    // Set up ifaliasreq
    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, getName().c_str(), IFNAMSIZ - 1);
    std::memcpy(&ifra.ifra_addr, &addr, sizeof(addr));
    std::memcpy(&ifra.ifra_mask, &mask, sizeof(mask));
    std::memcpy(&ifra.ifra_broadaddr, &broadcast, sizeof(broadcast));

    // Add the alias address
    bool result = (ioctl(sock, SIOCAIFADDR, &ifra) == 0);
    close(sock);
    return result;
  }

  // Default implementation for removeAddress that can be used by all interfaces
  bool Interface::removeAddress() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    // Remove the primary address
    if (ioctl(sock, SIOCDIFADDR, &ifr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool Interface::removeAliasAddress(const libfreebsdnet::types::Address &address) {
    if (!address.isValid()) {
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    // Get sockaddr from Address object
    struct sockaddr_in addr = address.getSockaddrIn();
    if (addr.sin_family != AF_INET) {
      close(sock);
      return false;
    }

    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, getName().c_str(), IFNAMSIZ - 1);
    std::memcpy(&ifra.ifra_addr, &addr, sizeof(addr));

    // Remove the alias address
    bool result = (ioctl(sock, SIOCDIFADDR, &ifra) == 0);
    close(sock);
    return result;
  }

  bool Interface::setAliasAddress(const std::string &addressString) {
    libfreebsdnet::types::Address address(addressString);
    return setAliasAddress(address);
  }

  bool Interface::removeAliasAddress(const std::string &addressString) {
    libfreebsdnet::types::Address address(addressString);
    return removeAliasAddress(address);
  }

  std::vector<std::string> Interface::getGroups() const {
    std::vector<std::string> groups;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return groups;
    }

    struct ifgroupreq ifgr;
    std::memset(&ifgr, 0, sizeof(ifgr));
    std::strncpy(ifgr.ifgr_name, getName().c_str(), IFNAMSIZ - 1);

    // First get the size
    if (ioctl(sock, SIOCGIFGROUP, &ifgr) < 0) {
      close(sock);
      return groups;
    }

    if (ifgr.ifgr_len > 0) {
      // Allocate buffer for groups
      std::vector<char> buffer(ifgr.ifgr_len);
      ifgr.ifgr_groups = reinterpret_cast<struct ifg_req *>(buffer.data());

      // Get the groups
      if (ioctl(sock, SIOCGIFGROUP, &ifgr) == 0) {
        int numGroups = ifgr.ifgr_len / sizeof(struct ifg_req);
        for (int i = 0; i < numGroups; i++) {
          groups.push_back(std::string(ifgr.ifgr_groups[i].ifgrq_group));
        }
      }
    }

    close(sock);
    return groups;
  }

  bool Interface::addToGroup(const std::string &groupName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifgroupreq ifgr;
    std::memset(&ifgr, 0, sizeof(ifgr));
    std::strncpy(ifgr.ifgr_name, getName().c_str(), IFNAMSIZ - 1);
    std::strncpy(ifgr.ifgr_group, groupName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCAIFGROUP, &ifgr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool Interface::removeFromGroup(const std::string &groupName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifgroupreq ifgr;
    std::memset(&ifgr, 0, sizeof(ifgr));
    std::strncpy(ifgr.ifgr_name, getName().c_str(), IFNAMSIZ - 1);
    std::strncpy(ifgr.ifgr_group, groupName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCDIFGROUP, &ifgr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  // Media methods
  int Interface::getMedia() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return ifmr.ifm_current;
  }

  bool Interface::setMedia(int media) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, getName().c_str(), IFNAMSIZ - 1);
    ifmr.ifm_current = media;

    if (ioctl(sock, SIOCSIFMEDIA, &ifmr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  int Interface::getMediaStatus() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return ifmr.ifm_status;
  }

  int Interface::getActiveMedia() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return ifmr.ifm_active;
  }

  std::vector<int> Interface::getSupportedMedia() const {
    std::vector<int> media;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return media;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return media;
    }

    if (ifmr.ifm_count > 0) {
      // Allocate buffer for media types
      std::vector<char> buffer(ifmr.ifm_count * sizeof(int));
      ifmr.ifm_ulist = reinterpret_cast<int *>(buffer.data());

      // Get the media types
      if (ioctl(sock, SIOCGIFMEDIA, &ifmr) == 0) {
        for (int i = 0; i < ifmr.ifm_count; i++) {
          media.push_back(ifmr.ifm_ulist[i]);
        }
      }
    }

    close(sock);
    return media;
  }

  // Capabilities methods
  uint32_t Interface::getCapabilities() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFCAP, &ifr) < 0) {
      close(sock);
      return 0;
    }

    close(sock);
    return ifr.ifr_reqcap;
  }

  bool Interface::setCapabilities(uint32_t capabilities) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);
    ifr.ifr_reqcap = capabilities;

    if (ioctl(sock, SIOCSIFCAP, &ifr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  uint32_t Interface::getEnabledCapabilities() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFCAP, &ifr) < 0) {
      close(sock);
      return 0;
    }

    close(sock);
    return ifr.ifr_curcap;
  }

  bool Interface::enableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current | capabilities);
  }

  bool Interface::disableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current & ~capabilities);
  }

  // Physical address methods
  bool Interface::setPhysicalAddress(const std::string &address) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, getName().c_str(), IFNAMSIZ - 1);

    // Parse IP address
    struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(&ifra.ifra_addr);
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, address.c_str(), &sin->sin_addr) != 1) {
      close(sock);
      return false;
    }

    if (ioctl(sock, SIOCSIFPHYADDR, &ifra) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool Interface::deletePhysicalAddress() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCDIFPHYADDR, &ifr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  // Clone methods
  bool Interface::createClone(const std::string &cloneName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, cloneName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::vector<std::string> Interface::getCloners() const {
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

  // MAC address methods
  std::string Interface::getMacAddress() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "";
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
      close(sock);
      return "";
    }

    close(sock);
    return "";
  }

  bool Interface::setMacAddress(const std::string &macAddress) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    // Parse MAC address
    if (sscanf(macAddress.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &ifr.ifr_addr.sa_data[0], &ifr.ifr_addr.sa_data[1],
               &ifr.ifr_addr.sa_data[2], &ifr.ifr_addr.sa_data[3],
               &ifr.ifr_addr.sa_data[4], &ifr.ifr_addr.sa_data[5]) != 6) {
      close(sock);
      return false;
    }

    ifr.ifr_addr.sa_family = AF_LINK;

    if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool Interface::destroy() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      if (pImpl) {
        pImpl->lastError = "Failed to create socket";
      }
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    if (pImpl) {
      std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    }
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
      if (pImpl) {
        pImpl->lastError = "Failed to destroy interface: " + std::string(strerror(errno));
      }
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

} // namespace libfreebsdnet::interface
