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
#include <jail.h>
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

  // Base class method implementations
  std::string WirelessInterface::getName() const {
    return Interface::getName();
  }
  unsigned int WirelessInterface::getIndex() const {
    return Interface::getIndex();
  }
  std::vector<Flag> WirelessInterface::getFlags() const {
    return Interface::getFlags();
  }
  bool WirelessInterface::setFlags(int flags) {
    return Interface::setFlags(flags);
  }
  std::string WirelessInterface::getLastError() const {
    return Interface::getLastError();
  }

  bool WirelessInterface::bringUp() { return Interface::bringUp(); }
  bool WirelessInterface::bringDown() { return Interface::bringDown(); }
  bool WirelessInterface::isUp() const { return Interface::isUp(); }

  int WirelessInterface::getMtu() const { return Interface::getMtu(); }
  bool WirelessInterface::setMtu(int mtu) { return Interface::setMtu(mtu); }

  int WirelessInterface::getFib() const { return Interface::getFib(); }
  bool WirelessInterface::setFib(int fib) { return Interface::setFib(fib); }

  int WirelessInterface::getMedia() const { return Interface::getMedia(); }

  bool WirelessInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int WirelessInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int WirelessInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> WirelessInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t WirelessInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool WirelessInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t WirelessInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool WirelessInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool WirelessInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  std::vector<std::string> WirelessInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool WirelessInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool WirelessInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
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

  std::string WirelessInterface::getVnetJailName() const {
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
    return Interface::setPhysicalAddress(address);
  }

  bool WirelessInterface::deletePhysicalAddress() {
    return Interface::deletePhysicalAddress();
  }

  bool WirelessInterface::createClone(const std::string &cloneName) {
    return Interface::createClone(cloneName);
  }

  std::vector<std::string> WirelessInterface::getCloners() const {
    return Interface::getCloners();
  }

  std::string WirelessInterface::getMacAddress() const {
    return Interface::getMacAddress();
  }

  bool WirelessInterface::setMacAddress(const std::string &macAddress) {
    return Interface::setMacAddress(macAddress);
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
