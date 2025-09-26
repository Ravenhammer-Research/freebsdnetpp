/**
 * @file interface/pfsync.cpp
 * @brief PFSYNC interface implementation
 * @details Implementation of PFSYNC interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/pfsync.hpp>
#include <net/if.h>
#include <net/if_mib.h>
#include <net/if_pfsync.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class PfsyncInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;
    std::string syncDevice;
    std::string syncPeer;
    int maxUpdates;
    bool defer;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags), maxUpdates(128),
          defer(false) {}
  };

  PfsyncInterface::PfsyncInterface(const std::string &name, unsigned int index,
                                   int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}

  PfsyncInterface::~PfsyncInterface() = default;

  std::string PfsyncInterface::getName() const { return pImpl->name; }

  unsigned int PfsyncInterface::getIndex() const { return pImpl->index; }

  InterfaceType PfsyncInterface::getType() const {
    return InterfaceType::PFSYNC;
  }

  int PfsyncInterface::getFlags() const { return pImpl->flags; }

  bool PfsyncInterface::setFlags(int flags) {
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

  bool PfsyncInterface::bringUp() {
    int newFlags = pImpl->flags | IFF_UP;
    return setFlags(newFlags);
  }

  bool PfsyncInterface::bringDown() {
    int newFlags = pImpl->flags & ~IFF_UP;
    return setFlags(newFlags);
  }

  bool PfsyncInterface::isUp() const { return (pImpl->flags & IFF_UP) != 0; }

  int PfsyncInterface::getMtu() const {
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

  bool PfsyncInterface::setMtu(int mtu) {
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

  std::string PfsyncInterface::getLastError() const { return pImpl->lastError; }

  std::string PfsyncInterface::getSyncInterface() const {
    return pImpl->syncDevice;
  }

  bool PfsyncInterface::setSyncInterface(const std::string &interfaceName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct pfsyncreq req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.pfsyncr_syncdev, interfaceName.c_str(), IFNAMSIZ - 1);
    req.pfsyncr_maxupdates = pImpl->maxUpdates;
    req.pfsyncr_defer = pImpl->defer ? PFSYNCF_DEFER : 0;

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_data = reinterpret_cast<caddr_t>(&req);

    if (ioctl(sock, SIOCSETPFSYNC, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set PFSYNC interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->syncDevice = interfaceName;
    close(sock);
    return true;
  }

  std::string PfsyncInterface::getSyncPeer() const { return pImpl->syncPeer; }

  bool PfsyncInterface::setSyncPeer(const std::string &peerAddress) {
    pImpl->syncPeer = peerAddress;
    return true; // Peer setting would require specific PFSYNC ioctls
  }

  int PfsyncInterface::getMaxUpdates() const { return pImpl->maxUpdates; }

  bool PfsyncInterface::setMaxUpdates(int maxUpdates) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct pfsyncreq req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.pfsyncr_syncdev, pImpl->syncDevice.c_str(), IFNAMSIZ - 1);
    req.pfsyncr_maxupdates = maxUpdates;
    req.pfsyncr_defer = pImpl->defer ? PFSYNCF_DEFER : 0;

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_data = reinterpret_cast<caddr_t>(&req);

    if (ioctl(sock, SIOCSETPFSYNC, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set PFSYNC max updates: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->maxUpdates = maxUpdates;
    close(sock);
    return true;
  }

  int PfsyncInterface::getFib() const {
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

  bool PfsyncInterface::setFib(int fib) {
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

  int PfsyncInterface::getMedia() const {
    // PFSYNC interfaces don't have media
    return -1;
  }

  bool PfsyncInterface::setMedia(int media) {
    (void)media; // Suppress unused parameter warning
    pImpl->lastError = "PFSYNC interfaces don't support media";
    return false;
  }

  int PfsyncInterface::getMediaStatus() const {
    // PFSYNC interfaces don't have media
    return -1;
  }

  int PfsyncInterface::getActiveMedia() const {
    // PFSYNC interfaces don't have media
    return -1;
  }

  std::vector<int> PfsyncInterface::getSupportedMedia() const {
    // PFSYNC interfaces don't have media
    return {};
  }

  uint32_t PfsyncInterface::getCapabilities() const {
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

  bool PfsyncInterface::setCapabilities(uint32_t capabilities) {
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

  uint32_t PfsyncInterface::getEnabledCapabilities() const {
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

  bool PfsyncInterface::enableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current | capabilities);
  }

  bool PfsyncInterface::disableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current & ~capabilities);
  }

  std::vector<std::string> PfsyncInterface::getGroups() const {
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

  bool PfsyncInterface::addToGroup(const std::string &groupName) {
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

  bool PfsyncInterface::removeFromGroup(const std::string &groupName) {
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

  int PfsyncInterface::getVnet() const {
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

  bool PfsyncInterface::setVnet(int vnetId) {
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

  bool PfsyncInterface::reclaimFromVnet() {
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

  bool PfsyncInterface::setPhysicalAddress(const std::string &address) {
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

  bool PfsyncInterface::deletePhysicalAddress() {
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

  bool PfsyncInterface::createClone(const std::string &cloneName) {
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

  std::vector<std::string> PfsyncInterface::getCloners() const {
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

  std::string PfsyncInterface::getMacAddress() const {
    // PFSYNC interfaces typically don't have MAC addresses
    return "";
  }

  bool PfsyncInterface::setMacAddress(const std::string &macAddress) {
    (void)macAddress; // Suppress unused parameter warning
    // PFSYNC interfaces typically don't support MAC addresses
    pImpl->lastError = "PFSYNC interfaces do not support MAC addresses";
    return false;
  }

  int PfsyncInterface::getTunnelFib() const {
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

  bool PfsyncInterface::setTunnelFib(int fib) {
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

  bool PfsyncInterface::destroy() {
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
