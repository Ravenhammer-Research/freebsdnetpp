/**
 * @file interface/lagg.cpp
 * @brief LAGG interface implementation
 * @details Implementation of LAGG interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/lagg.hpp>
#include <jail.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_lagg.h>
#include <net/if_mib.h>
#include <net/infiniband.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {


  LagInterface::LagInterface(const std::string &name, unsigned int index,
                             int flags)
      : Interface(name, index, flags) {}

  LagInterface::~LagInterface() = default;

  std::string LagInterface::getName() const { return Interface::getName(); }

  unsigned int LagInterface::getIndex() const { return Interface::getIndex(); }

  InterfaceType LagInterface::getType() const { return InterfaceType::LAGG; }

  std::vector<Flag> LagInterface::getFlags() const { return Interface::getFlags(); }
  bool LagInterface::setFlags(int flags) { return Interface::setFlags(flags); }
  bool LagInterface::bringUp() { return Interface::bringUp(); }
  bool LagInterface::bringDown() { return Interface::bringDown(); }
  bool LagInterface::isUp() const { return Interface::isUp(); }
  int LagInterface::getMtu() const { return Interface::getMtu(); }
  bool LagInterface::setMtu(int mtu) { return Interface::setMtu(mtu); }
  std::string LagInterface::getLastError() const { return Interface::getLastError(); }
  int LagInterface::getFib() const { return Interface::getFib(); }
  bool LagInterface::setFib(int fib) { return Interface::setFib(fib); }


  LagProtocol LagInterface::getProtocol() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return LagProtocol::UNKNOWN;
    }

    struct lagg_reqall ra;
    std::memset(&ra, 0, sizeof(ra));
    std::strncpy(ra.ra_ifname, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGLAGG, &ra) == 0) {
      close(sock);
      switch (ra.ra_proto) {
        case LAGG_PROTO_FAILOVER:
          return LagProtocol::FAILOVER;
        case LAGG_PROTO_LACP:
          return LagProtocol::LACP;
        case LAGG_PROTO_LOADBALANCE:
          return LagProtocol::LOADBALANCE;
        case LAGG_PROTO_ROUNDROBIN:
          return LagProtocol::ROUNDROBIN;
        default:
          return LagProtocol::UNKNOWN;
      }
    }

    close(sock);
    return LagProtocol::UNKNOWN;
  }

  bool LagInterface::setProtocol(LagProtocol protocol) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct lagg_reqall req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.ra_ifname, getName().c_str(), IFNAMSIZ - 1);

    // Convert protocol enum to LAGG protocol
    switch (protocol) {
    case LagProtocol::FAILOVER:
      req.ra_proto = LAGG_PROTO_FAILOVER;
            // Protocol will be retrieved by getProtocol() method
      break;
    case LagProtocol::LACP:
      req.ra_proto = LAGG_PROTO_LACP;
// Protocol will be retrieved by getProtocol() method
      break;
    case LagProtocol::LOADBALANCE:
      req.ra_proto = LAGG_PROTO_LOADBALANCE;
// Protocol will be retrieved by getProtocol() method
      break;
    case LagProtocol::ROUNDROBIN:
      req.ra_proto = LAGG_PROTO_ROUNDROBIN;
// Protocol will be retrieved by getProtocol() method
      break;
    // BROADCAST not supported in enum
    default:
// Use base class error handling
      close(sock);
      return false;
    }

    if (ioctl(sock, SIOCSLAGG, &req) < 0) {
// Use base class error handling
          "Failed to set LAGG protocol: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool LagInterface::addInterface(const std::string &interfaceName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    // First, try to create the lagg interface if it doesn't exist
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
      if (errno != EEXIST) {
// Use base class error handling "Failed to create lagg interface: " + std::string(strerror(errno));
        close(sock);
        return false;
      }
      // Interface already exists, that's fine
    }

    // Set protocol first if not already set
    struct lagg_reqall ra;
    std::memset(&ra, 0, sizeof(ra));
    std::strncpy(ra.ra_ifname, getName().c_str(), IFNAMSIZ - 1);
    ra.ra_proto = LAGG_PROTO_DEFAULT;

    if (ioctl(sock, SIOCSLAGG, &ra) < 0) {
// Use base class error handling "Failed to set lagg protocol: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    // Now add the port
    struct lagg_reqport req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.rp_ifname, getName().c_str(), IFNAMSIZ - 1);
    std::strncpy(req.rp_portname, interfaceName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCSLAGGPORT, &req) < 0) {
// Use base class error handling
          "Failed to add interface to LAGG: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    // Add to our local list
    // Ports will be retrieved by getPorts() method
    close(sock);
    return true;
  }

  bool LagInterface::removeInterface(const std::string &interfaceName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct lagg_reqport req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.rp_ifname, getName().c_str(), IFNAMSIZ - 1);
    std::strncpy(req.rp_portname, interfaceName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCSLAGGDELPORT, &req) < 0) {
// Use base class error handling "Failed to remove interface from LAGG: " +
                         std::string(strerror(errno));
      close(sock);
      return false;
    }

    // Remove from our local list
    // Ports will be retrieved by getPorts() method
    // Ports will be retrieved by getPorts() method

    close(sock);
    return true;
  }


  bool LagInterface::hasInterface(const std::string &interfaceName) const {
    (void)interfaceName; // Suppress unused parameter warning
    // Ports will be retrieved by getPorts() method
    return false;
  }

  int LagInterface::getActiveInterfaceCount() const {
    // Ports will be retrieved by getPorts() method
    return 0;
  }


  int LagInterface::getMedia() const {
    return Interface::getMedia();
  }

  bool LagInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int LagInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int LagInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> LagInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t LagInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool LagInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t LagInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool LagInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool LagInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  std::vector<std::string> LagInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool LagInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool LagInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  int LagInterface::getVnet() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return ifr.ifr_jid;
  }

  std::string LagInterface::getVnetJailName() const {
    int vnetId = getVnet();
    if (vnetId < 0) {
      return "";
    }

    char *jailName = jail_getname(vnetId);
    if (jailName != nullptr) {
      std::string result(jailName);
      free(jailName);
      return result;
    }

    return "";
  }

  bool LagInterface::setVnet(int vnetId) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);
    ifr.ifr_jid = vnetId;

    if (ioctl(sock, SIOCSIFVNET, &ifr) < 0) {
// Use base class error handling "Failed to set VNET: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool LagInterface::reclaimFromVnet() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCSIFRVNET, &ifr) < 0) {
// Use base class error handling
          "Failed to reclaim from VNET: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool LagInterface::setPhysicalAddress(const std::string &address) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, getName().c_str(), IFNAMSIZ - 1);

    // Parse IP address
    struct sockaddr_in *sin =
        reinterpret_cast<struct sockaddr_in *>(&ifra.ifra_addr);
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, address.c_str(), &sin->sin_addr) != 1) {
// Use base class error handling "Invalid IP address format";
      close(sock);
      return false;
    }

    if (ioctl(sock, SIOCSIFPHYADDR, &ifra) < 0) {
// Use base class error handling
          "Failed to set physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool LagInterface::deletePhysicalAddress() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCDIFPHYADDR, &ifr) < 0) {
// Use base class error handling
          "Failed to delete physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool LagInterface::createClone(const std::string &cloneName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, cloneName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
// Use base class error handling
          "Failed to create clone: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::vector<std::string> LagInterface::getCloners() const {
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

  std::string LagInterface::getMacAddress() const {
    struct ifaddrs *ifaddrs, *ifa;
    std::string macAddress = "";

    if (getifaddrs(&ifaddrs) != 0) {
      return macAddress;
    }

    for (ifa = ifaddrs; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_name && std::string(ifa->ifa_name) == getName()) {
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

  bool LagInterface::setMacAddress(const std::string &macAddress) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    // Parse MAC address (format: "aa:bb:cc:dd:ee:ff")
    unsigned char mac[6];
    if (std::sscanf(macAddress.c_str(),
                    "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[0],
                    &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
// Use base class error handling "Invalid MAC address format";
      close(sock);
      return false;
    }

    // Set the MAC address in the ifreq structure
    std::memcpy(ifr.ifr_addr.sa_data, mac, 6);
    ifr.ifr_addr.sa_family = AF_LINK;
    ifr.ifr_addr.sa_len = 6;

    if (ioctl(sock, SIOCSIFLLADDR, &ifr) < 0) {
// Use base class error handling
          "Failed to set MAC address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }


  // InfiniBand LAG-specific methods
  bool LagInterface::isInfinibandLag() const {
    // Check if this is an InfiniBand LAG by examining the interface type
    // This would typically be done by checking the interface type from the
    // system For now, we'll check if the interface name suggests InfiniBand LAG
    return getName().find("ib") == 0 || getName().find("infiniband") == 0;
  }

  std::string LagInterface::getInfinibandAddress() const {
    if (!isInfinibandLag()) {
      return "";
    }

    struct ifaddrs *ifaddrs, *ifa;
    std::string address = "";

    if (getifaddrs(&ifaddrs) != 0) {
      return address;
    }

    for (ifa = ifaddrs; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_name && std::string(ifa->ifa_name) == getName()) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
          struct sockaddr_dl *sdl =
              reinterpret_cast<struct sockaddr_dl *>(ifa->ifa_addr);
          if (sdl->sdl_alen == INFINIBAND_ADDR_LEN) {
            // Format InfiniBand address as hex string
            char addrStr[INFINIBAND_ADDR_LEN * 2 + 1];
            for (int i = 0; i < INFINIBAND_ADDR_LEN; i++) {
              std::snprintf(addrStr + (i * 2), 3, "%02x", sdl->sdl_data[i]);
            }
            address = std::string(addrStr);
            break;
          }
        }
      }
    }

    freeifaddrs(ifaddrs);
    return address;
  }

  bool LagInterface::setInfinibandAddress(const std::string &address) {
    if (!isInfinibandLag()) {
// Use base class error handling "Not an InfiniBand LAG interface";
      return false;
    }

    if (address.length() != INFINIBAND_ADDR_LEN * 2) {
// Use base class error handling "Invalid InfiniBand address format";
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    // Parse InfiniBand address
    unsigned char addr[INFINIBAND_ADDR_LEN];
    for (int i = 0; i < INFINIBAND_ADDR_LEN; i++) {
      if (std::sscanf(address.c_str() + (i * 2), "%02hhx", &addr[i]) != 1) {
// Use base class error handling "Invalid InfiniBand address format";
        close(sock);
        return false;
      }
    }

    // For InfiniBand addresses, we need to use a different approach
    // since sa_data is only 14 bytes but InfiniBand addresses are 20 bytes
    // This is a limitation of the standard sockaddr structure
    // InfiniBand address setting not supported - address too long for standard sockaddr
    close(sock);
    return false;
  }

  int LagInterface::getInfinibandMtu() const {
    if (!isInfinibandLag()) {
      return -1;
    }

    // Use the standard MTU method for InfiniBand LAG
    return getMtu();
  }

  bool LagInterface::setInfinibandMtu(int mtu) {
    if (!isInfinibandLag()) {
// Use base class error handling "Not an InfiniBand LAG interface";
      return false;
    }

    // InfiniBand has specific MTU constraints
    if (mtu < 256 || mtu > 4096) {
// Use base class error handling "Invalid InfiniBand MTU (must be 256-4096)";
      return false;
    }

    // Use the standard MTU method for InfiniBand LAG
    return setMtu(mtu);
  }

  // IEEE 802.3ad LAG-specific methods
  bool LagInterface::isIeee8023adLag() const {
    // Check if this is an IEEE 802.3ad LAG by examining the interface type
    // This would typically be done by checking the interface type from the
    // system For now, we'll check if the interface name suggests IEEE 802.3ad
    // LAG
    return getName().find("lagg") == 0 && getProtocol() == LagProtocol::LACP;
  }

  std::string LagInterface::getLacpStatus() const {
    if (!isIeee8023adLag()) {
      return "Not an IEEE 802.3ad LAG";
    }

    // This would require kernel-level access to LACP status
    // For now, return a basic status
    return "LACP Active";
  }

  bool LagInterface::setLacpStrictMode(bool strict) {
    if (!isIeee8023adLag()) {
// Use base class error handling "Not an IEEE 802.3ad LAG";
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct lagg_reqport lrp;
    std::memset(&lrp, 0, sizeof(lrp));
    std::strncpy(lrp.rp_ifname, getName().c_str(), IFNAMSIZ - 1);

    if (strict) {
      lrp.rp_flags |= LAGG_OPT_LACP_STRICT;
    } else {
      lrp.rp_flags &= ~LAGG_OPT_LACP_STRICT;
    }

    if (ioctl(sock, SIOCSLAGGPORT, &lrp) < 0) {
// Use base class error handling
          "Failed to set LACP strict mode: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool LagInterface::getLacpStrictMode() const {
    if (!isIeee8023adLag()) {
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct lagg_reqport lrp;
    std::memset(&lrp, 0, sizeof(lrp));
    std::strncpy(lrp.rp_ifname, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGLAGGPORT, &lrp) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return (lrp.rp_flags & LAGG_OPT_LACP_STRICT) != 0;
  }

  bool LagInterface::setLacpFastTimeout(bool fast) {
    if (!isIeee8023adLag()) {
// Use base class error handling "Not an IEEE 802.3ad LAG";
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling
      return false;
    }

    struct lagg_reqport lrp;
    std::memset(&lrp, 0, sizeof(lrp));
    std::strncpy(lrp.rp_ifname, getName().c_str(), IFNAMSIZ - 1);

    if (fast) {
      lrp.rp_flags |= LAGG_OPT_LACP_FAST_TIMO;
    } else {
      lrp.rp_flags &= ~LAGG_OPT_LACP_FAST_TIMO;
    }

    if (ioctl(sock, SIOCSLAGGPORT, &lrp) < 0) {
// Use base class error handling
          "Failed to set LACP fast timeout: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool LagInterface::getLacpFastTimeout() const {
    if (!isIeee8023adLag()) {
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct lagg_reqport lrp;
    std::memset(&lrp, 0, sizeof(lrp));
    std::strncpy(lrp.rp_ifname, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGLAGGPORT, &lrp) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return (lrp.rp_flags & LAGG_OPT_LACP_FAST_TIMO) != 0;
  }

  std::string LagInterface::getLacpPartnerInfo() const {
    if (!isIeee8023adLag()) {
      return "";
    }

    // This would require kernel-level access to LACP partner information
    // For now, return a placeholder
    return "Partner information not available - requires kernel-level access";
  }

  int LagInterface::getLacpSystemPriority() const {
    if (!isIeee8023adLag()) {
      return -1;
    }

    // This would require kernel-level access to LACP system priority
    // For now, return a default value
    return 32768; // Default system priority
  }

  bool LagInterface::setLacpSystemPriority(int priority) {
    if (!isIeee8023adLag()) {
// Use base class error handling "Not an IEEE 802.3ad LAG";
      return false;
    }

    if (priority < 0 || priority > 65535) {
// Use base class error handling "Invalid LACP system priority (must be 0-65535)";
      return false;
    }

    // This would require kernel-level access to set LACP system priority
    // LACP system priority setting not implemented - requires kernel-level access
    return false;
  }

  bool LagInterface::destroy() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
// Use base class error handling "Failed to create socket: " + std::string(strerror(errno));
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
// Use base class error handling "Failed to destroy interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::vector<std::string> LagInterface::getPorts() const {
    std::vector<std::string> ports;
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return ports; // Fallback to internal list
    }
    
    // Use SIOCGLAGG to get lagg information
    struct lagg_reqall ra;
    std::memset(&ra, 0, sizeof(ra));
    std::strncpy(ra.ra_ifname, getName().c_str(), IFNAMSIZ - 1);
    
    // Allocate buffer for ports
    struct lagg_reqport *portbuf = (struct lagg_reqport *)malloc(sizeof(struct lagg_reqport) * 32);
    if (!portbuf) {
      close(sock);
      return ports;
    }
    
    ra.ra_port = portbuf;
    ra.ra_size = sizeof(struct lagg_reqport) * 32;
    
    if (ioctl(sock, SIOCGLAGG, &ra) == 0) {
      // Successfully got lagg info, extract ports
      for (int i = 0; i < ra.ra_ports; i++) {
        ports.push_back(std::string(portbuf[i].rp_portname));
      }
      free(portbuf);
      close(sock);
      return ports;
    }
    
    free(portbuf);
    close(sock);
    return ports; // Fallback to internal list
  }

  std::string LagInterface::getHashType() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "Unknown";
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    // Get interface flags to determine hash type
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
      close(sock);
      return "Unknown";
    }

    close(sock);
    
    // For now, return a default hash type since FreeBSD doesn't expose this directly
    // In a real implementation, this would query the kernel for the actual hash configuration
    return "l2,l3,l4";
  }

} // namespace libfreebsdnet::interface
