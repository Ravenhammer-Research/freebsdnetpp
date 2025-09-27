/**
 * @file interface/carp.cpp
 * @brief CARP interface implementation
 * @details Implementation of CARP interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/carp.hpp>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip_carp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class CarpInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;
    int vhid;
    CarpState state;
    int advbase;
    int advskew;
    std::string peer;
    std::string peer6;
    std::string key;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags), vhid(-1),
          state(CarpState::INIT), advbase(1), advskew(0) {}
  };

  CarpInterface::CarpInterface(const std::string &name, unsigned int index,
                               int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}

  CarpInterface::~CarpInterface() = default;

  // Base class method implementations
  std::string CarpInterface::getName() const { return Interface::getName(); }
  unsigned int CarpInterface::getIndex() const { return Interface::getIndex(); }
  InterfaceType CarpInterface::getType() const { return InterfaceType::CARP; }
  std::vector<Flag> CarpInterface::getFlags() const {
    return Interface::getFlags();
  }
  bool CarpInterface::setFlags(int flags) { return Interface::setFlags(flags); }
  bool CarpInterface::bringUp() { return Interface::bringUp(); }
  bool CarpInterface::bringDown() { return Interface::bringDown(); }
  bool CarpInterface::isUp() const { return Interface::isUp(); }

  int CarpInterface::getMtu() const { return Interface::getMtu(); }
  bool CarpInterface::setMtu(int mtu) { return Interface::setMtu(mtu); }
  std::string CarpInterface::getLastError() const {
    return Interface::getLastError();
  }

  int CarpInterface::getFib() const { return Interface::getFib(); }
  bool CarpInterface::setFib(int fib) { return Interface::setFib(fib); }

  int CarpInterface::getVhid() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCGVH, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return carpr.carpr_vhid;
  }

  bool CarpInterface::setVhid(int vhid) {
    if (vhid < 1 || vhid > 255) {
      pImpl->lastError = "Invalid VHID: " + std::to_string(vhid);
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

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    carpr.carpr_vhid = vhid;
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCSVH, &ifr) < 0) {
      pImpl->lastError = "Failed to set VHID: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->vhid = vhid;
    close(sock);
    return true;
  }

  CarpState CarpInterface::getState() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return CarpState::INIT;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCGVH, &ifr) < 0) {
      close(sock);
      return CarpState::INIT;
    }

    close(sock);

    switch (carpr.carpr_state) {
    case 0: // INIT
      return CarpState::INIT;
    case 1: // BACKUP
      return CarpState::BACKUP;
    case 2: // MASTER
      return CarpState::MASTER;
    default:
      return CarpState::INIT;
    }
  }

  int CarpInterface::getAdvBase() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCGVH, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return carpr.carpr_advbase;
  }

  bool CarpInterface::setAdvBase(int advbase) {
    if (advbase < 1 || advbase > 255) {
      pImpl->lastError =
          "Invalid advertisement base: " + std::to_string(advbase);
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

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    carpr.carpr_advbase = advbase;
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCSVH, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set advertisement base: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->advbase = advbase;
    close(sock);
    return true;
  }

  int CarpInterface::getAdvSkew() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCGVH, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return carpr.carpr_advskew;
  }

  bool CarpInterface::setAdvSkew(int advskew) {
    if (advskew < 0 || advskew > 255) {
      pImpl->lastError =
          "Invalid advertisement skew: " + std::to_string(advskew);
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

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    carpr.carpr_advskew = advskew;
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCSVH, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set advertisement skew: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->advskew = advskew;
    close(sock);
    return true;
  }

  std::string CarpInterface::getPeerAddress() const {
    // CARP peer addresses are not stored in the carpreq structure
    // This would need to be implemented using different methods
    return pImpl->peer;
  }

  bool CarpInterface::setPeerAddress(const std::string &peer) {
    // CARP peer addresses are not stored in the carpreq structure
    // This would need to be implemented using different methods
    pImpl->peer = peer;
    return true;
  }

  std::string CarpInterface::getPeerAddress6() const {
    // CARP peer addresses are not stored in the carpreq structure
    // This would need to be implemented using different methods
    return pImpl->peer6;
  }

  bool CarpInterface::setPeerAddress6(const std::string &peer6) {
    // CARP peer addresses are not stored in the carpreq structure
    // This would need to be implemented using different methods
    pImpl->peer6 = peer6;
    return true;
  }

  std::string CarpInterface::getKey() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "";
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCGVH, &ifr) < 0) {
      close(sock);
      return "";
    }

    close(sock);
    return std::string(reinterpret_cast<const char *>(carpr.carpr_key));
  }

  bool CarpInterface::setKey(const std::string &key) {
    if (key.length() > CARP_KEY_LEN) {
      pImpl->lastError = "Key too long: " + std::to_string(key.length());
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

    struct carpreq carpr;
    std::memset(&carpr, 0, sizeof(carpr));
    std::strncpy(reinterpret_cast<char *>(carpr.carpr_key), key.c_str(),
                 CARP_KEY_LEN - 1);
    ifr.ifr_data = reinterpret_cast<caddr_t>(&carpr);

    if (ioctl(sock, SIOCSVH, &ifr) < 0) {
      pImpl->lastError = "Failed to set key: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->key = key;
    close(sock);
    return true;
  }

  bool CarpInterface::isValid() const { return getVhid() > 0; }

  int CarpInterface::getMedia() const {
    // CARP interfaces don't have media
    return -1;
  }

  bool CarpInterface::setMedia(int media) {
    (void)media; // Suppress unused parameter warning
    pImpl->lastError = "CARP interfaces don't support media";
    return false;
  }

  int CarpInterface::getMediaStatus() const {
    // CARP interfaces don't have media
    return -1;
  }

  int CarpInterface::getActiveMedia() const {
    // CARP interfaces don't have media
    return -1;
  }

  std::vector<int> CarpInterface::getSupportedMedia() const {
    // CARP interfaces don't have media
    return {};
  }

  uint32_t CarpInterface::getCapabilities() const {
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

  bool CarpInterface::setCapabilities(uint32_t capabilities) {
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

  uint32_t CarpInterface::getEnabledCapabilities() const {
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

  bool CarpInterface::enableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current | capabilities);
  }

  bool CarpInterface::disableCapabilities(uint32_t capabilities) {
    uint32_t current = getEnabledCapabilities();
    return setCapabilities(current & ~capabilities);
  }

  std::vector<std::string> CarpInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool CarpInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool CarpInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  bool CarpInterface::setPhysicalAddress(const std::string &address) {
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

  bool CarpInterface::deletePhysicalAddress() {
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

  bool CarpInterface::createClone(const std::string &cloneName) {
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

  std::vector<std::string> CarpInterface::getCloners() const {
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

  std::string CarpInterface::getMacAddress() const {
    // CARP interfaces typically don't have MAC addresses
    return "";
  }

  bool CarpInterface::setMacAddress(const std::string &macAddress) {
    (void)macAddress; // Suppress unused parameter warning
    // CARP interfaces typically don't support MAC addresses
    pImpl->lastError = "CARP interfaces do not support MAC addresses";
    return false;
  }

  bool CarpInterface::destroy() {
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
