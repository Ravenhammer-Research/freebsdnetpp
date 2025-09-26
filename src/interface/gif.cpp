/**
 * @file interface/gif.cpp
 * @brief GIF tunnel interface implementation
 * @details Implementation of GIF tunnel interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/gif.hpp>
#include <jail.h>
#include <net/if.h>
#include <net/if_gif.h>
#include <net/if_mib.h>
#include <net/if_private.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  GifInterface::GifInterface(const std::string &name, unsigned int index, int flags)
      : TunnelInterface(name, index, flags) {}

  GifInterface::~GifInterface() = default;

  InterfaceType GifInterface::getType() const {
    return InterfaceType::GIF;
  }

  int GifInterface::getProtocol() const {
    // Default to IPv4 for now
    return 4;
  }

  bool GifInterface::setProtocol(int protocol) {
    if (protocol < 0 || protocol > 255) {
      // Use base class error handling
      return false;
    }
    // Protocol setting would require specific GIF ioctls
    return true;
  }

  std::string GifInterface::getLocalAddress() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "";
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFPSRCADDR, &ifr) < 0) {
      close(sock);
      return "";
    }

    const struct sockaddr_in *sin = reinterpret_cast<const struct sockaddr_in*>(&ifr.ifr_addr);
    if (sin->sin_family != AF_INET) {
      close(sock);
      return "";
    }

    char addr_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &sin->sin_addr, addr_str, INET_ADDRSTRLEN) == nullptr) {
      close(sock);
      return "";
    }

    close(sock);
    return std::string(addr_str);
  }

  bool GifInterface::setLocalAddress(const std::string &address) {
    // Validate IP address format
    struct in_addr addr;
    if (inet_pton(AF_INET, address.c_str(), &addr) != 1) {
      // Use base class error handling
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      // Use base class error handling
      return false;
    }

    struct in_aliasreq addreq;
    std::memset(&addreq, 0, sizeof(addreq));
    std::strncpy(addreq.ifra_name, getName().c_str(), IFNAMSIZ - 1);
    
    // Set the local address (source)
    struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in*>(&addreq.ifra_addr);
    sin->sin_family = AF_INET;
    sin->sin_addr = addr;

    if (ioctl(sock, SIOCSIFPHYADDR, &addreq) < 0) {
      // Use base class error handling
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::string GifInterface::getRemoteAddress() const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return "";
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFPDSTADDR, &ifr) < 0) {
      close(sock);
      return "";
    }

    const struct sockaddr_in *sin = reinterpret_cast<const struct sockaddr_in*>(&ifr.ifr_addr);
    if (sin->sin_family != AF_INET) {
      close(sock);
      return "";
    }

    char addr_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &sin->sin_addr, addr_str, INET_ADDRSTRLEN) == nullptr) {
      close(sock);
      return "";
    }

    close(sock);
    return std::string(addr_str);
  }

  bool GifInterface::setRemoteAddress(const std::string &address) {
    // Validate IP address format
    struct in_addr addr;
    if (inet_pton(AF_INET, address.c_str(), &addr) != 1) {
      // Use base class error handling
      return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      // Use base class error handling
      return false;
    }

    struct in_aliasreq addreq;
    std::memset(&addreq, 0, sizeof(addreq));
    std::strncpy(addreq.ifra_name, getName().c_str(), IFNAMSIZ - 1);
    
    // Set the remote address (destination)
    struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in*>(&addreq.ifra_dstaddr);
    sin->sin_family = AF_INET;
    sin->sin_addr = addr;

    if (ioctl(sock, SIOCSIFPHYADDR, &addreq) < 0) {
      // Use base class error handling
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  int GifInterface::getTtl() const {
    // Default TTL for GIF
    return 64;
  }

  bool GifInterface::setTtl(int ttl) {
    if (ttl < 0 || ttl > 255) {
      // Use base class error handling
      return false;
    }
    // TTL setting would require specific GIF ioctls
    return true;
  }

  bool GifInterface::isPmtuDiscoveryEnabled() const {
    // Default to enabled
    return true;
  }

  bool GifInterface::setPmtuDiscovery(bool enabled) {
    (void)enabled; // Suppress unused parameter warning
    // PMTU discovery setting would require specific GIF ioctls
    return true;
  }

  int GifInterface::getTunnelFib() const {
    return TunnelInterface::getTunnelFib();
  }

  bool GifInterface::setTunnelFib(int fib) {
    return TunnelInterface::setTunnelFib(fib);
  }

  // Group management methods (call base class)
  std::vector<std::string> GifInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool GifInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool GifInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  // Base class method implementations (call base class)
  int GifInterface::getMedia() const {
    return Interface::getMedia();
  }

  bool GifInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int GifInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int GifInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> GifInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t GifInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool GifInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t GifInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool GifInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool GifInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  bool GifInterface::setPhysicalAddress(const std::string &address) {
    return Interface::setPhysicalAddress(address);
  }

  bool GifInterface::deletePhysicalAddress() {
    return Interface::deletePhysicalAddress();
  }

  bool GifInterface::createClone(const std::string &cloneName) {
    return Interface::createClone(cloneName);
  }

  std::vector<std::string> GifInterface::getCloners() const {
    return Interface::getCloners();
  }

  std::string GifInterface::getMacAddress() const {
    return Interface::getMacAddress();
  }

  bool GifInterface::setMacAddress(const std::string &macAddress) {
    return Interface::setMacAddress(macAddress);
  }

  // VNET interface methods
  int GifInterface::getVnet() const {
    // VNET functionality not implemented yet
    return -1;
  }

  std::string GifInterface::getVnetJailName() const {
    // VNET functionality not implemented yet
    return "";
  }

  bool GifInterface::setVnet(int vnetId) {
    (void)vnetId; // Suppress unused parameter warning
    // VNET functionality not implemented yet
    return false;
  }

  bool GifInterface::reclaimFromVnet() {
    // VNET functionality not implemented yet
    return false;
  }

} // namespace libfreebsdnet::interface
