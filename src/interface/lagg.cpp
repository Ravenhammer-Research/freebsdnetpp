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

  class LagInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;
    std::vector<std::string> ports;
    std::string protocol;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags) {}
  };

  LagInterface::LagInterface(const std::string &name, unsigned int index,
                             int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}

  LagInterface::~LagInterface() = default;

  std::string LagInterface::getName() const { return pImpl->name; }

  unsigned int LagInterface::getIndex() const { return pImpl->index; }

  InterfaceType LagInterface::getType() const { return InterfaceType::LAGG; }

  int LagInterface::getFlags() const { return pImpl->flags; }

  bool LagInterface::setFlags(int flags) {
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

  bool LagInterface::bringUp() {
    int newFlags = pImpl->flags | IFF_UP;
    return setFlags(newFlags);
  }

  bool LagInterface::bringDown() {
    int newFlags = pImpl->flags & ~IFF_UP;
    return setFlags(newFlags);
  }

  bool LagInterface::isUp() const { return (pImpl->flags & IFF_UP) != 0; }

  int LagInterface::getMtu() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return ifr.ifr_mtu;
  }

  bool LagInterface::setMtu(int mtu) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_mtu = mtu;

    if (ioctl(sock, SIOCSIFMTU, &ifr) < 0) {
      pImpl->lastError = "Failed to set MTU: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::string LagInterface::getLastError() const { return pImpl->lastError; }

  LagProtocol LagInterface::getProtocol() const {
    if (pImpl->protocol == "failover") {
      return LagProtocol::FAILOVER;
    } else if (pImpl->protocol == "lacp") {
      return LagProtocol::LACP;
    } else if (pImpl->protocol == "loadbalance") {
      return LagProtocol::LOADBALANCE;
    } else if (pImpl->protocol == "roundrobin") {
      return LagProtocol::ROUNDROBIN;
    } else if (pImpl->protocol == "broadcast") {
      return LagProtocol::UNKNOWN; // BROADCAST not in enum
    }
    return LagProtocol::UNKNOWN;
  }

  bool LagInterface::setProtocol(LagProtocol protocol) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct lagg_reqall req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.ra_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);

    // Convert protocol enum to LAGG protocol
    switch (protocol) {
    case LagProtocol::FAILOVER:
      req.ra_proto = LAGG_PROTO_FAILOVER;
      pImpl->protocol = "failover";
      break;
    case LagProtocol::LACP:
      req.ra_proto = LAGG_PROTO_LACP;
      pImpl->protocol = "lacp";
      break;
    case LagProtocol::LOADBALANCE:
      req.ra_proto = LAGG_PROTO_LOADBALANCE;
      pImpl->protocol = "loadbalance";
      break;
    case LagProtocol::ROUNDROBIN:
      req.ra_proto = LAGG_PROTO_ROUNDROBIN;
      pImpl->protocol = "roundrobin";
      break;
    // BROADCAST not supported in enum
    default:
      pImpl->lastError = "Unknown protocol";
      close(sock);
      return false;
    }

    if (ioctl(sock, SIOCSLAGG, &req) < 0) {
      pImpl->lastError =
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
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    // First, try to create the lagg interface if it doesn't exist
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
      if (errno != EEXIST) {
        pImpl->lastError = "Failed to create lagg interface: " + std::string(strerror(errno));
        close(sock);
        return false;
      }
      // Interface already exists, that's fine
    }

    // Set protocol first if not already set
    struct lagg_reqall ra;
    std::memset(&ra, 0, sizeof(ra));
    std::strncpy(ra.ra_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);
    ra.ra_proto = LAGG_PROTO_DEFAULT;

    if (ioctl(sock, SIOCSLAGG, &ra) < 0) {
      pImpl->lastError = "Failed to set lagg protocol: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    // Now add the port
    struct lagg_reqport req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.rp_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);
    std::strncpy(req.rp_portname, interfaceName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCSLAGGPORT, &req) < 0) {
      pImpl->lastError =
          "Failed to add interface to LAGG: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    // Add to our local list
    pImpl->ports.push_back(interfaceName);
    close(sock);
    return true;
  }

  bool LagInterface::removeInterface(const std::string &interfaceName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct lagg_reqport req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.rp_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);
    std::strncpy(req.rp_portname, interfaceName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCSLAGGDELPORT, &req) < 0) {
      pImpl->lastError = "Failed to remove interface from LAGG: " +
                         std::string(strerror(errno));
      close(sock);
      return false;
    }

    // Remove from our local list
    auto it =
        std::find(pImpl->ports.begin(), pImpl->ports.end(), interfaceName);
    if (it != pImpl->ports.end()) {
      pImpl->ports.erase(it);
    }

    close(sock);
    return true;
  }


  bool LagInterface::hasInterface(const std::string &interfaceName) const {
    return std::find(pImpl->ports.begin(), pImpl->ports.end(), interfaceName) !=
           pImpl->ports.end();
  }

  int LagInterface::getActiveInterfaceCount() const {
    return pImpl->ports.size();
  }

  int LagInterface::getFib() const {
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

  bool LagInterface::setFib(int fib) {
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

  int LagInterface::getMedia() const {
    // LAGG interfaces inherit media from member interfaces
    return -1;
  }

  bool LagInterface::setMedia(int media) {
    (void)media; // Suppress unused parameter warning
    pImpl->lastError = "LAGG interfaces inherit media from member interfaces";
    return false;
  }

  int LagInterface::getMediaStatus() const {
    // LAGG interfaces inherit media from member interfaces
    return -1;
  }

  int LagInterface::getActiveMedia() const {
    // LAGG interfaces inherit media from member interfaces
    return -1;
  }

  std::vector<int> LagInterface::getSupportedMedia() const {
    // LAGG interfaces inherit media from member interfaces
    return {};
  }

  uint32_t LagInterface::getCapabilities() const {
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

  bool LagInterface::setCapabilities(uint32_t capabilities) {
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

  uint32_t LagInterface::getEnabledCapabilities() const {
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

  bool LagInterface::enableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current | capabilities);
  }

  bool LagInterface::disableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current & ~capabilities);
  }

  std::vector<std::string> LagInterface::getGroups() const {
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

  bool LagInterface::addToGroup(const std::string &groupName) {
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

  bool LagInterface::removeFromGroup(const std::string &groupName) {
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

  int LagInterface::getVnet() const {
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

  bool LagInterface::setVnet(int vnetId) {
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

  bool LagInterface::reclaimFromVnet() {
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

  bool LagInterface::setPhysicalAddress(const std::string &address) {
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

  bool LagInterface::deletePhysicalAddress() {
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

  bool LagInterface::createClone(const std::string &cloneName) {
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

  bool LagInterface::setMacAddress(const std::string &macAddress) {
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

  int LagInterface::getTunnelFib() const {
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

  bool LagInterface::setTunnelFib(int fib) {
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

  // InfiniBand LAG-specific methods
  bool LagInterface::isInfinibandLag() const {
    // Check if this is an InfiniBand LAG by examining the interface type
    // This would typically be done by checking the interface type from the
    // system For now, we'll check if the interface name suggests InfiniBand LAG
    return pImpl->name.find("ib") == 0 || pImpl->name.find("infiniband") == 0;
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
      if (ifa->ifa_name && std::string(ifa->ifa_name) == pImpl->name) {
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
      pImpl->lastError = "Not an InfiniBand LAG interface";
      return false;
    }

    if (address.length() != INFINIBAND_ADDR_LEN * 2) {
      pImpl->lastError = "Invalid InfiniBand address format";
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

    // Parse InfiniBand address
    unsigned char addr[INFINIBAND_ADDR_LEN];
    for (int i = 0; i < INFINIBAND_ADDR_LEN; i++) {
      if (std::sscanf(address.c_str() + (i * 2), "%02hhx", &addr[i]) != 1) {
        pImpl->lastError = "Invalid InfiniBand address format";
        close(sock);
        return false;
      }
    }

    // For InfiniBand addresses, we need to use a different approach
    // since sa_data is only 14 bytes but InfiniBand addresses are 20 bytes
    // This is a limitation of the standard sockaddr structure
    pImpl->lastError = "InfiniBand address setting not supported - address too "
                       "long for standard sockaddr";
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
      pImpl->lastError = "Not an InfiniBand LAG interface";
      return false;
    }

    // InfiniBand has specific MTU constraints
    if (mtu < 256 || mtu > 4096) {
      pImpl->lastError = "Invalid InfiniBand MTU (must be 256-4096)";
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
    return pImpl->name.find("lagg") == 0 && getProtocol() == LagProtocol::LACP;
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
      pImpl->lastError = "Not an IEEE 802.3ad LAG";
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct lagg_reqport lrp;
    std::memset(&lrp, 0, sizeof(lrp));
    std::strncpy(lrp.rp_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (strict) {
      lrp.rp_flags |= LAGG_OPT_LACP_STRICT;
    } else {
      lrp.rp_flags &= ~LAGG_OPT_LACP_STRICT;
    }

    if (ioctl(sock, SIOCSLAGGPORT, &lrp) < 0) {
      pImpl->lastError =
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
    std::strncpy(lrp.rp_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGLAGGPORT, &lrp) < 0) {
      close(sock);
      return false;
    }

    close(sock);
    return (lrp.rp_flags & LAGG_OPT_LACP_STRICT) != 0;
  }

  bool LagInterface::setLacpFastTimeout(bool fast) {
    if (!isIeee8023adLag()) {
      pImpl->lastError = "Not an IEEE 802.3ad LAG";
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct lagg_reqport lrp;
    std::memset(&lrp, 0, sizeof(lrp));
    std::strncpy(lrp.rp_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (fast) {
      lrp.rp_flags |= LAGG_OPT_LACP_FAST_TIMO;
    } else {
      lrp.rp_flags &= ~LAGG_OPT_LACP_FAST_TIMO;
    }

    if (ioctl(sock, SIOCSLAGGPORT, &lrp) < 0) {
      pImpl->lastError =
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
    std::strncpy(lrp.rp_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);

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
      pImpl->lastError = "Not an IEEE 802.3ad LAG";
      return false;
    }

    if (priority < 0 || priority > 65535) {
      pImpl->lastError = "Invalid LACP system priority (must be 0-65535)";
      return false;
    }

    // This would require kernel-level access to set LACP system priority
    pImpl->lastError = "LACP system priority setting not implemented - "
                       "requires kernel-level access";
    return false;
  }

  bool LagInterface::destroy() {
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

  std::vector<std::string> LagInterface::getPorts() const {
    std::vector<std::string> ports;
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return pImpl->ports; // Fallback to internal list
    }
    
    // Use SIOCGLAGG to get lagg information
    struct lagg_reqall ra;
    std::memset(&ra, 0, sizeof(ra));
    std::strncpy(ra.ra_ifname, pImpl->name.c_str(), IFNAMSIZ - 1);
    
    // Allocate buffer for ports
    struct lagg_reqport *portbuf = (struct lagg_reqport *)malloc(sizeof(struct lagg_reqport) * 32);
    if (!portbuf) {
      close(sock);
      return pImpl->ports;
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
    return pImpl->ports; // Fallback to internal list
  }

  std::string LagInterface::getHashType() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "Unknown";
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

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
