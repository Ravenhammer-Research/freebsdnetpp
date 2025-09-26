/**
 * @file netlink/manager.cpp
 * @brief Netlink manager implementation
 * @details Implementation of netlink interface management functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <atomic>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netlink/manager.hpp>
#include <netlink/netlink.h>
#include <netlink/netlink_route.h>
#include <netlink/netlink_snl.h>
#include <netlink/netlink_snl_route.h>
#include <sys/ioctl.h>
#include <sys/linker.h>
#include <sys/module.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace libfreebsdnet::netlink {

  class NetlinkManager::Impl {
  public:
    std::string lastError;
    std::atomic<bool> monitoring{false};
    std::thread monitorThread;
    NetlinkCallback callback;
    int netlinkSocket{-1};

    Impl() {
      // Check if netlink module is loaded
      if (modfind("netlink") == -1 && errno == ENOENT) {
        if (kldload("netlink") == -1) {
          lastError = "Netlink module not available";
          return;
        }
      }

      // Create netlink socket
      netlinkSocket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
      if (netlinkSocket < 0) {
        lastError =
            "Failed to create netlink socket: " + std::string(strerror(errno));
      }
    }

    ~Impl() {
      monitoring.store(false);
      if (monitorThread.joinable()) {
        monitorThread.join();
      }
      if (netlinkSocket >= 0) {
        close(netlinkSocket);
      }
    }
  };

  NetlinkManager::NetlinkManager() : pImpl(std::make_unique<Impl>()) {}

  NetlinkManager::~NetlinkManager() = default;

  bool NetlinkManager::isAvailable() const { return pImpl->netlinkSocket >= 0; }

  std::vector<NetlinkInterfaceInfo> NetlinkManager::getInterfaces() const {
    std::vector<NetlinkInterfaceInfo> interfaces;

    if (!isAvailable()) {
      return interfaces;
    }

    // For now, fall back to getifaddrs since netlink is complex
    // This provides compatibility without full netlink implementation
    struct ifaddrs *ifaddrs, *ifa;

    if (getifaddrs(&ifaddrs) != 0) {
      return interfaces;
    }

    for (ifa = ifaddrs; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_name && ifa->ifa_addr) {
        NetlinkInterfaceInfo info;
        info.name = std::string(ifa->ifa_name);
        info.index = if_nametoindex(ifa->ifa_name);
        info.flags = ifa->ifa_flags;
        info.change = 0; // Not available via getifaddrs

        // Get interface type
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock >= 0) {
          struct ifreq ifr;
          std::memset(&ifr, 0, sizeof(ifr));
          std::strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);

          if (ioctl(sock, SIOCGIFMTU, &ifr) == 0) {
            info.mtu = ifr.ifr_mtu;
          } else {
            info.mtu = 1500; // Default
          }

          close(sock);
        } else {
          info.mtu = 1500; // Default
        }

        // Get hardware address
        if (ifa->ifa_addr->sa_family == AF_LINK) {
          struct sockaddr_dl *sdl =
              reinterpret_cast<struct sockaddr_dl *>(ifa->ifa_addr);
          if (sdl->sdl_alen > 0) {
            char macStr[18];
            unsigned char *addr =
                reinterpret_cast<unsigned char *>(LLADDR(sdl));
            std::snprintf(macStr, sizeof(macStr),
                          "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1],
                          addr[2], addr[3], addr[4], addr[5]);
            info.hardwareAddress = std::string(macStr);
          }
        }

        // Determine interface type
        if (ifa->ifa_flags & IFF_LOOPBACK) {
          info.type = IFT_LOOP;
        } else if (ifa->ifa_flags & IFF_POINTOPOINT) {
          info.type = IFT_PPP;
        } else {
          info.type = IFT_ETHER;
        }

        // Determine operational state
        if (ifa->ifa_flags & IFF_UP) {
          info.operstate = "UP";
        } else {
          info.operstate = "DOWN";
        }

        interfaces.push_back(info);
      }
    }

    freeifaddrs(ifaddrs);
    return interfaces;
  }

  NetlinkInterfaceInfo
  NetlinkManager::getInterface(const std::string &name) const {
    NetlinkInterfaceInfo info;

    if (!isAvailable()) {
      return info;
    }

    auto interfaces = getInterfaces();
    for (const auto &iface : interfaces) {
      if (iface.name == name) {
        return iface;
      }
    }

    return info;
  }

  NetlinkInterfaceInfo NetlinkManager::getInterface(int index) const {
    NetlinkInterfaceInfo info;

    if (!isAvailable()) {
      return info;
    }

    auto interfaces = getInterfaces();
    for (const auto &iface : interfaces) {
      if (iface.index == index) {
        return iface;
      }
    }

    return info;
  }

  bool NetlinkManager::setInterfaceFlags(const std::string &name,
                                         uint32_t flags) {
    if (!isAvailable()) {
      pImpl->lastError = "Netlink not available";
      return false;
    }

    // For now, fall back to ioctl since netlink is complex
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_flags = flags;

    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set interface flags: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool NetlinkManager::setInterfaceMtu(const std::string &name, int mtu) {
    if (!isAvailable()) {
      pImpl->lastError = "Netlink not available";
      return false;
    }

    // For now, fall back to ioctl since netlink is complex
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);
    ifr.ifr_mtu = mtu;

    if (ioctl(sock, SIOCSIFMTU, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set interface MTU: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool NetlinkManager::startMonitoring(const NetlinkCallback &callback) {
    if (!isAvailable()) {
      pImpl->lastError = "Netlink not available";
      return false;
    }

    if (pImpl->monitoring.load()) {
      pImpl->lastError = "Already monitoring";
      return false;
    }

    pImpl->callback = callback;
    pImpl->monitoring.store(true);

    // For now, monitoring is not implemented
    // This would require complex netlink message parsing
    pImpl->lastError =
        "Netlink monitoring not implemented - requires complex message parsing";
    return false;
  }

  bool NetlinkManager::stopMonitoring() {
    if (!pImpl->monitoring.load()) {
      return true;
    }

    pImpl->monitoring.store(false);

    if (pImpl->monitorThread.joinable()) {
      pImpl->monitorThread.join();
    }

    return true;
  }

  std::string NetlinkManager::getLastError() const { return pImpl->lastError; }

} // namespace libfreebsdnet::netlink
