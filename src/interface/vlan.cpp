/**
 * @file interface/vlan.cpp
 * @brief VLAN interface implementation
 * @details Implementation of VLAN interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/vlan.hpp>
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

  class VlanInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags) {}
  };

  VlanInterface::VlanInterface(const std::string &name, unsigned int index,
                               int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}

  VlanInterface::~VlanInterface() = default;

  std::string VlanInterface::getName() const { return pImpl->name; }

  unsigned int VlanInterface::getIndex() const { return pImpl->index; }

  InterfaceType VlanInterface::getType() const { return InterfaceType::VLAN; }

  int VlanInterface::getFlags() const { return pImpl->flags; }

  bool VlanInterface::setFlags(int flags) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_flags = flags;

    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set interface flags: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->flags = flags;
    close(sock);
    return true;
  }

  bool VlanInterface::bringUp() {
    int newFlags = pImpl->flags | IFF_UP;
    return setFlags(newFlags);
  }

  bool VlanInterface::bringDown() {
    int newFlags = pImpl->flags & ~IFF_UP;
    return setFlags(newFlags);
  }

  bool VlanInterface::isUp() const { return (pImpl->flags & IFF_UP) != 0; }

  int VlanInterface::getMtu() const {
    return 1500; // Default MTU
  }

  bool VlanInterface::setMtu(int mtu) {
    (void)mtu;   // Suppress unused parameter warning
    return true; // Stub implementation
  }

  std::string VlanInterface::getLastError() const { return pImpl->lastError; }

  int VlanInterface::getVlanId() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct vlanreq vlr;
    std::memset(&vlr, 0, sizeof(vlr));
    ifr.ifr_data = reinterpret_cast<caddr_t>(&vlr);

    if (ioctl(sock, SIOCGETVLAN, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return vlr.vlr_tag;
  }

  bool VlanInterface::setVlanId(int vlanId) {
    if (vlanId < 1 || vlanId > 4094) {
      pImpl->lastError = "Invalid VLAN ID: " + std::to_string(vlanId);
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct vlanreq vlr;
    std::memset(&vlr, 0, sizeof(vlr));
    vlr.vlr_tag = vlanId;
    ifr.ifr_data = reinterpret_cast<caddr_t>(&vlr);

    if (ioctl(sock, SIOCSETVLAN, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set VLAN ID: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::string VlanInterface::getParentInterface() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "";
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct vlanreq vlr;
    std::memset(&vlr, 0, sizeof(vlr));
    ifr.ifr_data = reinterpret_cast<caddr_t>(&vlr);

    if (ioctl(sock, SIOCGETVLAN, &ifr) < 0) {
      close(sock);
      return "";
    }

    close(sock);
    return std::string(vlr.vlr_parent);
  }

  bool VlanInterface::setParentInterface(const std::string &parentInterface) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct vlanreq vlr;
    std::memset(&vlr, 0, sizeof(vlr));
    std::strncpy(vlr.vlr_parent, parentInterface.c_str(), IFNAMSIZ - 1);
    ifr.ifr_data = reinterpret_cast<caddr_t>(&vlr);

    if (ioctl(sock, SIOCSETVLAN, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set parent interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool VlanInterface::isValid() const {
    return getVlanId() > 0 && !getParentInterface().empty();
  }

  int VlanInterface::getFib() const {
    // Get FIB assignment using the correct FreeBSD ioctl (like ifconfig does)
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0; // Default FIB
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    int fib = 0;
    if (ioctl(sock, SIOCGIFFIB, &ifr) == 0) {
      fib = ifr.ifr_fib;
    }

    close(sock);
    return fib;
  }

  bool VlanInterface::setFib(int fib) {
    // Set FIB assignment using the correct FreeBSD ioctl (like ifconfig does)
    // Try AF_INET first, fall back to AF_LOCAL if that fails
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0 && errno == EAFNOSUPPORT) {
      sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    }
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket: " + std::string(strerror(errno));
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_fib = fib;

    if (ioctl(sock, SIOCSIFFIB, &ifr) < 0) {
      pImpl->lastError = "Failed to set FIB: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  int VlanInterface::getMedia() const {
    // VLAN interfaces inherit media from parent interface
    return -1;
  }

  bool VlanInterface::setMedia(int media) {
    (void)media; // Suppress unused parameter warning
    pImpl->lastError = "VLAN interfaces inherit media from parent interface";
    return false;
  }

  int VlanInterface::getMediaStatus() const {
    // VLAN interfaces inherit media from parent interface
    return -1;
  }

  int VlanInterface::getActiveMedia() const {
    // VLAN interfaces inherit media from parent interface
    return -1;
  }

  std::vector<int> VlanInterface::getSupportedMedia() const {
    // VLAN interfaces inherit media from parent interface
    return {};
  }

  uint32_t VlanInterface::getCapabilities() const {
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

  bool VlanInterface::setCapabilities(uint32_t capabilities) {
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

  uint32_t VlanInterface::getEnabledCapabilities() const {
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

  bool VlanInterface::enableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current | capabilities);
  }

  bool VlanInterface::disableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current & ~capabilities);
  }

  std::vector<std::string> VlanInterface::getGroups() const {
    std::vector<std::string> groups;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return groups;
    }

    struct ifgroupreq ifgr;
    std::memset(&ifgr, 0, sizeof(ifgr));
    std::strncpy(ifgr.ifgr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

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

  bool VlanInterface::addToGroup(const std::string &groupName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifgroupreq ifgr;
    std::memset(&ifgr, 0, sizeof(ifgr));
    std::strncpy(ifgr.ifgr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    std::strncpy(ifgr.ifgr_group, groupName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCAIFGROUP, &ifgr) < 0) {
      pImpl->lastError =
          "Failed to add to group: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool VlanInterface::removeFromGroup(const std::string &groupName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifgroupreq ifgr;
    std::memset(&ifgr, 0, sizeof(ifgr));
    std::strncpy(ifgr.ifgr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    std::strncpy(ifgr.ifgr_group, groupName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCDIFGROUP, &ifgr) < 0) {
      pImpl->lastError =
          "Failed to remove from group: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  int VlanInterface::getVnet() const {
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

  bool VlanInterface::setVnet(int vnetId) {
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

  bool VlanInterface::reclaimFromVnet() {
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

  bool VlanInterface::setPhysicalAddress(const std::string &address) {
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

  bool VlanInterface::deletePhysicalAddress() {
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

  bool VlanInterface::createClone(const std::string &cloneName) {
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

  std::vector<std::string> VlanInterface::getCloners() const {
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

  std::string VlanInterface::getMacAddress() const {
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

  bool VlanInterface::setMacAddress(const std::string &macAddress) {
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

  int VlanInterface::getTunnelFib() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGTUNFIB, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return ifr.ifr_fib;
  }

  bool VlanInterface::setTunnelFib(int fib) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_fib = fib;

    if (ioctl(sock, SIOCSTUNFIB, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set tunnel FIB: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool VlanInterface::destroy() {
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
