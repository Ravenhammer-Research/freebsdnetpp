/**
 * @file interface/wireless.cpp
 * @brief IEEE 802.11 wireless interface implementation
 * @details Implementation of IEEE 802.11 wireless interface management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <ifaddrs.h>
#include <interface/wireless.hpp>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_mib.h>
#include <net80211/ieee80211_freebsd.h>
#include <net80211/ieee80211_ioctl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <vector>

namespace libfreebsdnet::interface {

  class WirelessInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags) {}
  };

  WirelessInterface::WirelessInterface(const std::string &name,
                                       unsigned int index, int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}

  WirelessInterface::~WirelessInterface() = default;

  InterfaceType WirelessInterface::getType() const {
    return InterfaceType::WIRELESS;
  }

  bool WirelessInterface::isValid() const { return !pImpl->name.empty(); }

  std::string WirelessInterface::getName() const { return pImpl->name; }

  unsigned int WirelessInterface::getIndex() const { return pImpl->index; }

  int WirelessInterface::getFlags() const { return pImpl->flags; }

  bool WirelessInterface::setFlags(int flags) {
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
      pImpl->lastError = "Failed to set flags: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->flags = flags;
    close(sock);
    return true;
  }

  std::string WirelessInterface::getLastError() const {
    return pImpl->lastError;
  }

  bool WirelessInterface::bringUp() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_flags = pImpl->flags | IFF_UP;

    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
      pImpl->lastError =
          "Failed to bring up interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->flags = ifr.ifr_flags;
    close(sock);
    return true;
  }

  bool WirelessInterface::bringDown() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_flags = pImpl->flags & ~IFF_UP;

    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
      pImpl->lastError =
          "Failed to bring down interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    pImpl->flags = ifr.ifr_flags;
    close(sock);
    return true;
  }

  bool WirelessInterface::isUp() const { return (pImpl->flags & IFF_UP) != 0; }

  int WirelessInterface::getMtu() const {
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

  bool WirelessInterface::setMtu(int mtu) {
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

  int WirelessInterface::getFib() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFFIB, &ifr) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return ifr.ifr_fib;
  }

  bool WirelessInterface::setFib(int fib) {
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

  int WirelessInterface::getMedia() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return 0;
    }

    close(sock);
    return ifmr.ifm_current;
  }

  bool WirelessInterface::setMedia(int media) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_media = media;

    if (ioctl(sock, SIOCSIFMEDIA, &ifr) < 0) {
      pImpl->lastError = "Failed to set media: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  int WirelessInterface::getMediaStatus() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return 0;
    }

    close(sock);
    return ifmr.ifm_status;
  }

  int WirelessInterface::getActiveMedia() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return 0;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return 0;
    }

    close(sock);
    return ifmr.ifm_active;
  }

  std::vector<int> WirelessInterface::getSupportedMedia() const {
    std::vector<int> media;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return media;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return media;
    }

    if (ifmr.ifm_count > 0) {
      std::vector<int> media_list(ifmr.ifm_count);
      ifmr.ifm_ulist = media_list.data();

      if (ioctl(sock, SIOCGIFMEDIA, &ifmr) == 0) {
        media = media_list;
      }
    }

    close(sock);
    return media;
  }

  uint32_t WirelessInterface::getCapabilities() const {
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

  bool WirelessInterface::setCapabilities(uint32_t capabilities) {
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

  uint32_t WirelessInterface::getEnabledCapabilities() const {
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

  bool WirelessInterface::enableCapabilities(uint32_t capabilities) {
    return setCapabilities(getEnabledCapabilities() | capabilities);
  }

  bool WirelessInterface::disableCapabilities(uint32_t capabilities) {
    return setCapabilities(getEnabledCapabilities() & ~capabilities);
  }

  std::vector<std::string> WirelessInterface::getGroups() const {
    std::vector<std::string> groups;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return groups;
    }

    struct ifgroupreq ifgr;
    std::memset(&ifgr, 0, sizeof(ifgr));
    std::strncpy(ifgr.ifgr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFGROUP, &ifgr) < 0) {
      close(sock);
      return groups;
    }

    if (ifgr.ifgr_len > 0) {
      std::vector<char> buffer(ifgr.ifgr_len);
      ifgr.ifgr_groups = reinterpret_cast<struct ifg_req *>(buffer.data());

      if (ioctl(sock, SIOCGIFGROUP, &ifgr) == 0) {
        struct ifg_req *ifg =
            reinterpret_cast<struct ifg_req *>(ifgr.ifgr_groups);
        for (size_t i = 0; i < ifgr.ifgr_len / sizeof(struct ifg_req); i++) {
          if (ifg[i].ifgrq_member[0] != '\0') {
            groups.push_back(std::string(ifg[i].ifgrq_member));
          }
        }
      }
    }

    close(sock);
    return groups;
  }

  bool WirelessInterface::addToGroup(const std::string &groupName) {
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

  bool WirelessInterface::removeFromGroup(const std::string &groupName) {
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

  int WirelessInterface::getVnet() const {
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

  bool WirelessInterface::setVnet(int vnetId) {
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

  bool WirelessInterface::reclaimFromVnet() {
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

  bool WirelessInterface::setPhysicalAddress(const std::string &address) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, pImpl->name.c_str(), IFNAMSIZ - 1);

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

  bool WirelessInterface::deletePhysicalAddress() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCDIFPHYADDR, &ifra) < 0) {
      pImpl->lastError =
          "Failed to delete physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool WirelessInterface::createClone(const std::string &cloneName) {
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

  std::vector<std::string> WirelessInterface::getCloners() const {
    std::vector<std::string> cloners;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return cloners;
    }

    struct if_clonereq ifcr;
    std::memset(&ifcr, 0, sizeof(ifcr));

    if (ioctl(sock, SIOCIFGCLONERS, &ifcr) < 0) {
      close(sock);
      return cloners;
    }

    if (ifcr.ifcr_total > 0) {
      std::vector<char> buffer(ifcr.ifcr_total * IFNAMSIZ);
      ifcr.ifcr_count = ifcr.ifcr_total;
      // Note: ifcr_buf is not available in this version, skip for now

      if (ioctl(sock, SIOCIFGCLONERS, &ifcr) == 0) {
        char *ptr = buffer.data();
        for (int i = 0; i < ifcr.ifcr_count; i++) {
          cloners.push_back(std::string(ptr));
          ptr += IFNAMSIZ;
        }
      }
    }

    close(sock);
    return cloners;
  }

  std::string WirelessInterface::getMacAddress() const {
    std::string macAddress = "";
    struct ifaddrs *ifaddrs, *ifa;

    if (getifaddrs(&ifaddrs) == -1) {
      return macAddress;
    }

    for (ifa = ifaddrs; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == nullptr)
        continue;

      if (std::string(ifa->ifa_name) == pImpl->name &&
          ifa->ifa_addr->sa_family == AF_LINK) {
        struct sockaddr_dl *sdl =
            reinterpret_cast<struct sockaddr_dl *>(ifa->ifa_addr);
        if (sdl->sdl_alen == ETHER_ADDR_LEN) {
          char mac_str[18];
          std::snprintf(
              mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
              (unsigned char)LLADDR(sdl)[0], (unsigned char)LLADDR(sdl)[1],
              (unsigned char)LLADDR(sdl)[2], (unsigned char)LLADDR(sdl)[3],
              (unsigned char)LLADDR(sdl)[4], (unsigned char)LLADDR(sdl)[5]);
          macAddress = mac_str;
          break;
        }
      }
    }

    freeifaddrs(ifaddrs);
    return macAddress;
  }

  bool WirelessInterface::setMacAddress(const std::string &macAddress) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    unsigned char mac[ETHER_ADDR_LEN];
    if (std::sscanf(macAddress.c_str(),
                    "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[0],
                    &mac[1], &mac[2], &mac[3], &mac[4],
                    &mac[5]) != ETHER_ADDR_LEN) {
      pImpl->lastError = "Invalid MAC address format";
      close(sock);
      return false;
    }

    struct sockaddr_dl sdl;
    std::memset(&sdl, 0, sizeof(sdl));
    sdl.sdl_len = sizeof(sdl);
    sdl.sdl_family = AF_LINK;
    sdl.sdl_alen = ETHER_ADDR_LEN;
    std::memcpy(LLADDR(&sdl), mac, ETHER_ADDR_LEN);

    std::memcpy(&ifr.ifr_addr, &sdl,
                std::min(sizeof(sdl), sizeof(ifr.ifr_addr)));

    if (ioctl(sock, SIOCSIFLLADDR, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set MAC address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  int WirelessInterface::getTunnelFib() const {
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

  bool WirelessInterface::setTunnelFib(int fib) {
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

  // IEEE 802.11-specific methods
  int WirelessInterface::getChannel() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return -1;
    }

    struct ieee80211req req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.i_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    req.i_type = IEEE80211_IOC_CHANNEL;
    int channel = 0;
    req.i_data = &channel;
    req.i_len = sizeof(channel);

    if (ioctl(sock, SIOCG80211, &req) < 0) {
      close(sock);
      return -1;
    }

    close(sock);
    return channel;
  }

  bool WirelessInterface::setChannel(int channel) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ieee80211req req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.i_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    req.i_type = IEEE80211_IOC_CHANNEL;
    req.i_data = &channel;
    req.i_len = sizeof(channel);

    if (ioctl(sock, SIOCS80211, &req) < 0) {
      pImpl->lastError =
          "Failed to set channel: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::string WirelessInterface::getSsid() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "";
    }

    struct ieee80211req req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.i_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    req.i_type = IEEE80211_IOC_SSID;
    req.i_len = IEEE80211_NWID_LEN;
    char ssid_data[IEEE80211_NWID_LEN];
    req.i_data = ssid_data;

    if (ioctl(sock, SIOCG80211, &req) < 0) {
      close(sock);
      return "";
    }

    close(sock);
    return std::string(ssid_data, req.i_len);
  }

  bool WirelessInterface::setSsid(const std::string &ssid) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ieee80211req req;
    std::memset(&req, 0, sizeof(req));
    std::strncpy(req.i_name, pImpl->name.c_str(), IFNAMSIZ - 1);
    req.i_type = IEEE80211_IOC_SSID;
    req.i_len =
        std::min(ssid.length(), static_cast<size_t>(IEEE80211_NWID_LEN - 1));
    req.i_data = const_cast<char *>(ssid.c_str());

    if (ioctl(sock, SIOCS80211, &req) < 0) {
      pImpl->lastError = "Failed to set SSID: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::string WirelessInterface::getMode() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "unknown";
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      close(sock);
      return "unknown";
    }

    close(sock);

    // Check media flags like ifconfig does
    if (ifmr.ifm_current & IFM_IEEE80211_ADHOC) {
      if (ifmr.ifm_current & IFM_FLAG0) {
        return "ahdemo";
      } else {
        return "adhoc";
      }
    } else if (ifmr.ifm_current & IFM_IEEE80211_HOSTAP) {
      return "ap";
    } else if (ifmr.ifm_current & IFM_IEEE80211_MONITOR) {
      return "monitor";
    } else if (ifmr.ifm_current & IFM_IEEE80211_MBSS) {
      return "mesh";
    } else {
      return "sta";
    }
  }

  bool WirelessInterface::setMode(const std::string &mode) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifmediareq ifmr;
    std::memset(&ifmr, 0, sizeof(ifmr));
    std::strncpy(ifmr.ifm_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    // Get current media
    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0) {
      pImpl->lastError =
          "Failed to get current media: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    // Set media based on mode
    if (mode == "sta") {
      ifmr.ifm_current = IFM_IEEE80211; // Default station mode
    } else if (mode == "adhoc") {
      ifmr.ifm_current = IFM_IEEE80211 | IFM_IEEE80211_ADHOC;
    } else if (mode == "ap") {
      ifmr.ifm_current = IFM_IEEE80211 | IFM_IEEE80211_HOSTAP;
    } else if (mode == "monitor") {
      ifmr.ifm_current = IFM_IEEE80211 | IFM_IEEE80211_MONITOR;
    } else if (mode == "mesh") {
      ifmr.ifm_current = IFM_IEEE80211 | IFM_IEEE80211_MBSS;
    } else if (mode == "ahdemo") {
      ifmr.ifm_current = IFM_IEEE80211 | IFM_IEEE80211_ADHOC | IFM_FLAG0;
    } else {
      pImpl->lastError = "Invalid wireless mode: " + mode;
      close(sock);
      return false;
    }

    if (ioctl(sock, SIOCSIFMEDIA, &ifmr) < 0) {
      pImpl->lastError = "Failed to set mode: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  int WirelessInterface::getSignalStrength() const {
    // This would require more complex IEEE 802.11 ioctls
    // For now, return a placeholder
    return -1;
  }

  int WirelessInterface::getNoiseLevel() const {
    // This would require more complex IEEE 802.11 ioctls
    // For now, return a placeholder
    return -1;
  }

  std::vector<int> WirelessInterface::getSupportedRates() const {
    std::vector<int> rates;
    // This would require more complex IEEE 802.11 ioctls
    // For now, return empty vector
    return rates;
  }

  int WirelessInterface::getCurrentRate() const {
    // This would require more complex IEEE 802.11 ioctls
    // For now, return -1
    return -1;
  }

  bool WirelessInterface::isEncryptionEnabled() const {
    // This would require more complex IEEE 802.11 ioctls
    // For now, return false
    return false;
  }

  std::vector<std::string> WirelessInterface::getAvailableNetworks() const {
    std::vector<std::string> networks;
    // This would require scanning functionality
    // For now, return empty vector
    return networks;
  }

  bool WirelessInterface::destroy() {
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
