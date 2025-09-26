/**
 * @file interface/tunnel.cpp
 * @brief Tunnel interface implementation
 * @details Implementation of tunnel interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/tunnel.hpp>
#include <net/if.h>
#include <net/if_gif.h>
#include <net/if_mib.h>
#include <net/if_private.h>
#include <net/if_tap.h>
#include <net/if_tun.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class TunnelInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;
    std::string localEndpoint;
    std::string remoteEndpoint;
    int tunnelKey;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags), tunnelKey(-1) {}
  };

  TunnelInterface::TunnelInterface(const std::string &name, unsigned int index,
                                   int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}

  TunnelInterface::~TunnelInterface() = default;

  // Base class method implementations (call base class)
  int TunnelInterface::getMedia() const {
    return Interface::getMedia();
  }

  bool TunnelInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int TunnelInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int TunnelInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> TunnelInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t TunnelInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool TunnelInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t TunnelInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool TunnelInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool TunnelInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  bool TunnelInterface::setPhysicalAddress(const std::string &address) {
    return Interface::setPhysicalAddress(address);
  }

  bool TunnelInterface::deletePhysicalAddress() {
    return Interface::deletePhysicalAddress();
  }

  bool TunnelInterface::createClone(const std::string &cloneName) {
    return Interface::createClone(cloneName);
  }

  std::vector<std::string> TunnelInterface::getCloners() const {
    return Interface::getCloners();
  }

  std::string TunnelInterface::getMacAddress() const {
    return Interface::getMacAddress();
  }

  bool TunnelInterface::setMacAddress(const std::string &macAddress) {
    return Interface::setMacAddress(macAddress);
  }

  std::string TunnelInterface::getLocalEndpoint() const {
    return pImpl->localEndpoint;
  }

  bool TunnelInterface::setLocalEndpoint(const std::string &endpoint) {
    pImpl->localEndpoint = endpoint;
    return true; // Endpoint setting would require specific tunnel ioctls
  }

  std::string TunnelInterface::getRemoteEndpoint() const {
    return pImpl->remoteEndpoint;
  }

  bool TunnelInterface::setRemoteEndpoint(const std::string &endpoint) {
    pImpl->remoteEndpoint = endpoint;
    return true; // Endpoint setting would require specific tunnel ioctls
  }

  int TunnelInterface::getTunnelKey() const { return pImpl->tunnelKey; }

  bool TunnelInterface::setTunnelKey(int key) {
    pImpl->tunnelKey = key;
    return true; // Key setting would require specific tunnel ioctls
  }

  bool TunnelInterface::isConfigured() const {
    return !pImpl->localEndpoint.empty() && !pImpl->remoteEndpoint.empty() &&
           pImpl->tunnelKey >= 0;
  }








  int TunnelInterface::getTunnelFib() const {
    // Get tunnel FIB assignment using SIOCGTUNFIB
    // Try AF_INET first, fall back to AF_LOCAL if that fails
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0 && errno == EAFNOSUPPORT) {
      sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    }
    if (sock < 0) {
      return -1;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    int fib = -1;
    if (ioctl(sock, SIOCGTUNFIB, &ifr) == 0) {
      fib = ifr.ifr_fib;
    }

    close(sock);
    return fib;
  }

  bool TunnelInterface::setTunnelFib(int fib) {
    // Set tunnel FIB assignment using SIOCSTUNFIB
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

    if (ioctl(sock, SIOCSTUNFIB, &ifr) < 0) {
      pImpl->lastError =
          "Failed to set tunnel FIB: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool TunnelInterface::destroy() {
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
