/**
 * @file interface/bridge.cpp
 * @brief Bridge interface implementation
 * @details Implementation of bridge interface functionality
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
#include <interface/bridge.hpp>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_bridgevar.h>
#include <net/if_dl.h>
#include <net/if_mib.h>
#include <net/if_private.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <jail.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {


  BridgeInterface::BridgeInterface(const std::string &name, unsigned int index,
                                   int flags)
      : Interface(name, index, flags) {}

  BridgeInterface::~BridgeInterface() = default;

  std::string BridgeInterface::getName() const { return Interface::getName(); }

  unsigned int BridgeInterface::getIndex() const { return Interface::getIndex(); }

  InterfaceType BridgeInterface::getType() const {
    return InterfaceType::BRIDGE;
  }

  std::vector<Flag> BridgeInterface::getFlags() const { return Interface::getFlags(); }

  bool BridgeInterface::setFlags(int flags) {
    return Interface::setFlags(flags);
  }

  bool BridgeInterface::bringUp() {
    return Interface::bringUp();
  }

  bool BridgeInterface::bringDown() {
    return Interface::bringDown();
  }

  bool BridgeInterface::isUp() const { return Interface::isUp(); }

  int BridgeInterface::getMtu() const {
    return Interface::getMtu();
  }

  bool BridgeInterface::setMtu(int mtu) {
    return Interface::setMtu(mtu);
  }

  std::string BridgeInterface::getLastError() const { return Interface::getLastError(); }

  int BridgeInterface::getFib() const {
    return Interface::getFib();
  }

  bool BridgeInterface::setFib(int fib) {
    return Interface::setFib(fib);
  }

  bool BridgeInterface::addInterface(const std::string &interfaceName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      // Use base class error handling
      return false;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGADD;
    ifd.ifd_len = sizeof(struct ifbreq);

    struct ifbreq ifbr;
    std::memset(&ifbr, 0, sizeof(ifbr));
    std::strncpy(ifbr.ifbr_ifsname, interfaceName.c_str(), IFNAMSIZ - 1);
    ifd.ifd_data = &ifbr;

    if (ioctl(sock, SIOCSDRVSPEC, &ifd) < 0) {
      pImpl->lastError =
          "Failed to add interface to bridge: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool BridgeInterface::removeInterface(const std::string &interfaceName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      // Use base class error handling
      return false;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGDEL;
    ifd.ifd_len = sizeof(struct ifbreq);

    struct ifbreq ifbr;
    std::memset(&ifbr, 0, sizeof(ifbr));
    std::strncpy(ifbr.ifbr_ifsname, interfaceName.c_str(), IFNAMSIZ - 1);
    ifd.ifd_data = &ifbr;

    if (ioctl(sock, SIOCSDRVSPEC, &ifd) < 0) {
      pImpl->lastError = "Failed to remove interface from bridge: " +
                         std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::vector<std::string> BridgeInterface::getInterfaces() const {
    // Bridge interface enumeration requires complex FreeBSD ioctls
    // For now, return empty list
    return {};
  }

  bool BridgeInterface::hasInterface(const std::string &interfaceName) const {
    auto interfaces = getInterfaces();
    return std::find(interfaces.begin(), interfaces.end(), interfaceName) !=
           interfaces.end();
  }

  bool BridgeInterface::enableStp() {
    pImpl->lastError = "STP operations not implemented - requires specific "
                       "FreeBSD bridge ioctls";
    return false;
  }

  bool BridgeInterface::disableStp() {
    pImpl->lastError = "STP operations not implemented - requires specific "
                       "FreeBSD bridge ioctls";
    return false;
  }

  bool BridgeInterface::isStpEnabled() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGGIFFLGS;
    ifd.ifd_len = sizeof(struct ifbreq);

    struct ifbreq ifbr;
    std::memset(&ifbr, 0, sizeof(ifbr));
    ifd.ifd_data = &ifbr;

    bool stpEnabled = false;
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
      stpEnabled = (ifbr.ifbr_ifsflags & IFBIF_STP) != 0;
    }

    close(sock);
    return stpEnabled;
  }

  bool BridgeInterface::setPriority(uint16_t priority) {
    (void)priority; // Suppress unused parameter warning
    pImpl->lastError = "Bridge priority operations not implemented - requires "
                       "specific FreeBSD bridge ioctls";
    return false;
  }

  int BridgeInterface::getPriority() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGPARAM;
    ifd.ifd_len = sizeof(struct ifbropreq);
    ifd.ifd_data = malloc(ifd.ifd_len);
    
    int result = -1;
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
      struct ifbropreq *ifbrop = static_cast<struct ifbropreq*>(ifd.ifd_data);
      result = ifbrop->ifbop_priority;
    }
    
    free(ifd.ifd_data);
    close(sock);
    return result;
  }

  bool BridgeInterface::setAgingTime(int seconds) {
    (void)seconds; // Suppress unused parameter warning
    pImpl->lastError = "Bridge aging operations not implemented - requires "
                       "specific FreeBSD bridge ioctls";
    return false;
  }

  int BridgeInterface::getAgingTime() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGGTO;  // Get cache timeout
    ifd.ifd_len = sizeof(struct ifbrparam);

    struct ifbrparam param;
    std::memset(&param, 0, sizeof(param));
    ifd.ifd_data = &param;

    int agingTime = -1;
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
      agingTime = param.ifbrp_ctime;
    }

    close(sock);
    return agingTime;
  }


  int BridgeInterface::getMedia() const {
    return Interface::getMedia();
  }

  bool BridgeInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int BridgeInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int BridgeInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> BridgeInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t BridgeInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool BridgeInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t BridgeInterface::getEnabledCapabilities() const {
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

  bool BridgeInterface::enableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current | capabilities);
  }

  bool BridgeInterface::disableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current & ~capabilities);
  }

  std::vector<std::string> BridgeInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool BridgeInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool BridgeInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  int BridgeInterface::getVnet() const {
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

  std::string BridgeInterface::getVnetJailName() const {
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

  bool BridgeInterface::setVnet(int vnetId) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);
    ifr.ifr_jid = vnetId;

    if (ioctl(sock, SIOCSIFVNET, &ifr) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool BridgeInterface::reclaimFromVnet() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      // Use base class error handling
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCSIFRVNET, &ifr) < 0) {
      pImpl->lastError =
          "Failed to reclaim from VNET: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool BridgeInterface::setPhysicalAddress(const std::string &address) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      // Use base class error handling
      return false;
    }

    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    // Parse IP address
    struct sockaddr_in *sin =
        reinterpret_cast<struct sockaddr_in *>(&ifra.ifra_addr);
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, address.c_str(), &sin->sin_addr) != 1) {
      pImpl->lastError = "Invalid IP address format";
      close(sock);
      return false;
    }

    if (ioctl(sock, SIOCSIFPHYADDR, &ifra) < 0) {
      pImpl->lastError =
          "Failed to set physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool BridgeInterface::deletePhysicalAddress() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      // Use base class error handling
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCDIFPHYADDR, &ifr) < 0) {
      pImpl->lastError =
          "Failed to delete physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool BridgeInterface::createClone(const std::string &cloneName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      // Use base class error handling
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

  std::vector<std::string> BridgeInterface::getCloners() const {
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

  std::string BridgeInterface::getMacAddress() const {
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

  bool BridgeInterface::setMacAddress(const std::string &macAddress) {
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


  bool BridgeInterface::destroy() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket: " + std::string(strerror(errno));
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
      pImpl->lastError = "Failed to destroy interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  int BridgeInterface::getHelloTime() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGPARAM;
    ifd.ifd_len = sizeof(struct ifbropreq);
    ifd.ifd_data = malloc(ifd.ifd_len);
    
    int result = -1;
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
      struct ifbropreq *ifbrop = static_cast<struct ifbropreq*>(ifd.ifd_data);
      result = ifbrop->ifbop_hellotime;
    }
    
    free(ifd.ifd_data);
    close(sock);
    return result;
  }

  int BridgeInterface::getForwardDelay() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGPARAM;
    ifd.ifd_len = sizeof(struct ifbropreq);
    ifd.ifd_data = malloc(ifd.ifd_len);
    
    int result = -1;
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
      struct ifbropreq *ifbrop = static_cast<struct ifbropreq*>(ifd.ifd_data);
      result = ifbrop->ifbop_fwddelay;
    }
    
    free(ifd.ifd_data);
    close(sock);
    return result;
  }

  int BridgeInterface::getProtocol() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGPARAM;
    ifd.ifd_len = sizeof(struct ifbropreq);
    ifd.ifd_data = malloc(ifd.ifd_len);
    
    int result = -1;
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
      struct ifbropreq *ifbrop = static_cast<struct ifbropreq*>(ifd.ifd_data);
      result = ifbrop->ifbop_protocol;
    }
    
    free(ifd.ifd_data);
    close(sock);
    return result;
  }

  int BridgeInterface::getMaxAddresses() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGGCACHE;
    ifd.ifd_len = sizeof(struct ifbrparam);
    ifd.ifd_data = malloc(ifd.ifd_len);
    
    int result = -1;
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
      struct ifbrparam *ifbrp = static_cast<struct ifbrparam*>(ifd.ifd_data);
      result = ifbrp->ifbrp_csize;
    }
    
    free(ifd.ifd_data);
    close(sock);
    return result;
  }

  int BridgeInterface::getInterfaceCost(const std::string &interfaceName) const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    // Get all bridge members using BRDGGIFS
    struct ifbifconf members;
    char *buf = nullptr;
    int result = -1;
    
    for (size_t len = 8192; (buf = static_cast<char*>(realloc(buf, len))) != nullptr; len *= 2) {
      members.ifbic_buf = buf;
      members.ifbic_len = len;
      
      struct ifdrv ifd;
      std::memset(&ifd, 0, sizeof(ifd));
      std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
      ifd.ifd_cmd = BRDGGIFS;
      ifd.ifd_len = sizeof(members);
      ifd.ifd_data = &members;
      
      if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
        if ((members.ifbic_len + sizeof(*members.ifbic_req)) < len) {
          break;
        }
      } else {
        free(buf);
        close(sock);
        return -1;
      }
    }
    
    if (buf != nullptr) {
      // Search for the specific interface in the member list
      size_t member_count = members.ifbic_len / sizeof(*members.ifbic_req);
      for (size_t i = 0; i < member_count; i++) {
        struct ifbreq *member = &members.ifbic_req[i];
        if (std::strcmp(member->ifbr_ifsname, interfaceName.c_str()) == 0) {
          result = member->ifbr_path_cost;
          break;
        }
      }
      free(buf);
    }
    
    close(sock);
    return result;
  }

  int BridgeInterface::getRootPathCost() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifdrv ifd;
    std::memset(&ifd, 0, sizeof(ifd));
    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGPARAM;
    ifd.ifd_len = sizeof(struct ifbropreq);
    ifd.ifd_data = malloc(ifd.ifd_len);
    
    int result = -1;
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) == 0) {
      struct ifbropreq *ifbrop = static_cast<struct ifbropreq*>(ifd.ifd_data);
      result = ifbrop->ifbop_root_path_cost;
    }
    
    free(ifd.ifd_data);
    close(sock);
    return result;
  }

} // namespace libfreebsdnet::interface
