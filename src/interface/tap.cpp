/**
 * @file interface/tap.cpp
 * @brief TAP tunnel interface implementation
 * @details Implementation of TAP tunnel interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/tap.hpp>
#include <net/if.h>
#include <net/if_tap.h>
#include <net/if_mib.h>
#include <net/if_private.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class TapInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;
    int unit;
    int owner;
    int group;
    bool persistent;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags), unit(-1), owner(-1), group(-1), persistent(false) {}
  };

  TapInterface::TapInterface(const std::string &name, unsigned int index, int flags)
      : TunnelInterface(name, index, flags), pImpl(std::make_unique<Impl>(name, index, flags)) {}

  TapInterface::~TapInterface() = default;

  InterfaceType TapInterface::getType() const {
    return InterfaceType::TUNNEL; // TAP is a type of tunnel
  }

  int TapInterface::getUnit() const {
    return pImpl->unit;
  }

  bool TapInterface::setUnit(int unit) {
    if (unit < 0) {
      pImpl->lastError = "Invalid unit: must be non-negative";
      return false;
    }
    pImpl->unit = unit;
    return true; // Unit setting would require specific TAP ioctls
  }

  int TapInterface::getOwner() const {
    return pImpl->owner;
  }

  bool TapInterface::setOwner(int uid) {
    if (uid < 0) {
      pImpl->lastError = "Invalid UID: must be non-negative";
      return false;
    }
    pImpl->owner = uid;
    return true; // Owner setting would require specific TAP ioctls
  }

  int TapInterface::getGroup() const {
    return pImpl->group;
  }

  bool TapInterface::setGroup(int gid) {
    if (gid < 0) {
      pImpl->lastError = "Invalid GID: must be non-negative";
      return false;
    }
    pImpl->group = gid;
    return true; // Group setting would require specific TAP ioctls
  }

  bool TapInterface::isPersistent() const {
    return pImpl->persistent;
  }

  bool TapInterface::setPersistent(bool persistent) {
    pImpl->persistent = persistent;
    return true; // Persistent setting would require specific TAP ioctls
  }

  int TapInterface::getTunnelFib() const {
    throw std::runtime_error("TAP interfaces do not support tunnel FIB operations");
  }

  bool TapInterface::setTunnelFib(int fib) {
    (void)fib; // Suppress unused parameter warning
    throw std::runtime_error("TAP interfaces do not support tunnel FIB operations");
  }

  // Group management methods (call base class)
  std::vector<std::string> TapInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool TapInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool TapInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  // Base class method implementations (call base class)
  int TapInterface::getMedia() const {
    return Interface::getMedia();
  }

  bool TapInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int TapInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int TapInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> TapInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t TapInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool TapInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t TapInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool TapInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool TapInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  bool TapInterface::setPhysicalAddress(const std::string &address) {
    return Interface::setPhysicalAddress(address);
  }

  bool TapInterface::deletePhysicalAddress() {
    return Interface::deletePhysicalAddress();
  }

  bool TapInterface::createClone(const std::string &cloneName) {
    return Interface::createClone(cloneName);
  }

  std::vector<std::string> TapInterface::getCloners() const {
    return Interface::getCloners();
  }

  std::string TapInterface::getMacAddress() const {
    return Interface::getMacAddress();
  }

  bool TapInterface::setMacAddress(const std::string &macAddress) {
    return Interface::setMacAddress(macAddress);
  }

} // namespace libfreebsdnet::interface
