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
#include <net/if.h>
#include <net/if_gif.h>
#include <net/if_mib.h>
#include <net/if_private.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class GifInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;
    int protocol;
    std::string localAddress;
    std::string remoteAddress;
    int ttl;
    bool pmtuDiscovery;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags), protocol(4), ttl(64), pmtuDiscovery(true) {} // Default to IPv4
  };

  GifInterface::GifInterface(const std::string &name, unsigned int index, int flags)
      : TunnelInterface(name, index, flags), pImpl(std::make_unique<Impl>(name, index, flags)) {}

  GifInterface::~GifInterface() = default;

  InterfaceType GifInterface::getType() const {
    return InterfaceType::TUNNEL; // GIF is a type of tunnel
  }

  int GifInterface::getProtocol() const {
    return pImpl->protocol;
  }

  bool GifInterface::setProtocol(int protocol) {
    if (protocol < 0 || protocol > 255) {
      pImpl->lastError = "Invalid protocol: must be between 0 and 255";
      return false;
    }
    pImpl->protocol = protocol;
    return true; // Protocol setting would require specific GIF ioctls
  }

  std::string GifInterface::getLocalAddress() const {
    return pImpl->localAddress;
  }

  bool GifInterface::setLocalAddress(const std::string &address) {
    // Validate IP address format
    struct in_addr addr;
    if (inet_pton(AF_INET, address.c_str(), &addr) != 1) {
      pImpl->lastError = "Invalid IP address format";
      return false;
    }
    pImpl->localAddress = address;
    return true; // Local address setting would require specific GIF ioctls
  }

  std::string GifInterface::getRemoteAddress() const {
    return pImpl->remoteAddress;
  }

  bool GifInterface::setRemoteAddress(const std::string &address) {
    // Validate IP address format
    struct in_addr addr;
    if (inet_pton(AF_INET, address.c_str(), &addr) != 1) {
      pImpl->lastError = "Invalid IP address format";
      return false;
    }
    pImpl->remoteAddress = address;
    return true; // Remote address setting would require specific GIF ioctls
  }

  int GifInterface::getTtl() const {
    return pImpl->ttl;
  }

  bool GifInterface::setTtl(int ttl) {
    if (ttl < 0 || ttl > 255) {
      pImpl->lastError = "Invalid TTL: must be between 0 and 255";
      return false;
    }
    pImpl->ttl = ttl;
    return true; // TTL setting would require specific GIF ioctls
  }

  bool GifInterface::isPmtuDiscoveryEnabled() const {
    return pImpl->pmtuDiscovery;
  }

  bool GifInterface::setPmtuDiscovery(bool enabled) {
    pImpl->pmtuDiscovery = enabled;
    return true; // PMTU discovery setting would require specific GIF ioctls
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
