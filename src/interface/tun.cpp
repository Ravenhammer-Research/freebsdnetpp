/**
 * @file interface/tun.cpp
 * @brief TUN tunnel interface implementation
 * @details Implementation of TUN tunnel interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/tun.hpp>
#include <net/if.h>
#include <net/if_mib.h>
#include <net/if_private.h>
#include <net/if_tun.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class TunInterface::Impl {
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
        : name(name), index(index), flags(flags), unit(-1), owner(-1),
          group(-1), persistent(false) {}
  };

  TunInterface::TunInterface(const std::string &name, unsigned int index,
                             int flags)
      : TunnelInterface(name, index, flags),
        pImpl(std::make_unique<Impl>(name, index, flags)) {}

  TunInterface::~TunInterface() = default;

  InterfaceType TunInterface::getType() const {
    return InterfaceType::TUNNEL; // TUN is a type of tunnel
  }

  int TunInterface::getUnit() const { return pImpl->unit; }

  bool TunInterface::setUnit(int unit) {
    if (unit < 0) {
      pImpl->lastError = "Invalid unit: must be non-negative";
      return false;
    }
    pImpl->unit = unit;
    return true; // Unit setting would require specific TUN ioctls
  }

  int TunInterface::getOwner() const { return pImpl->owner; }

  bool TunInterface::setOwner(int uid) {
    if (uid < 0) {
      pImpl->lastError = "Invalid UID: must be non-negative";
      return false;
    }
    pImpl->owner = uid;
    return true; // Owner setting would require specific TUN ioctls
  }

  int TunInterface::getGroup() const { return pImpl->group; }

  bool TunInterface::setGroup(int gid) {
    if (gid < 0) {
      pImpl->lastError = "Invalid GID: must be non-negative";
      return false;
    }
    pImpl->group = gid;
    return true; // Group setting would require specific TUN ioctls
  }

  bool TunInterface::isPersistent() const { return pImpl->persistent; }

  bool TunInterface::setPersistent(bool persistent) {
    pImpl->persistent = persistent;
    return true; // Persistent setting would require specific TUN ioctls
  }

  int TunInterface::getTunnelFib() const {
    throw std::runtime_error(
        "TUN interfaces do not support tunnel FIB operations");
  }

  bool TunInterface::setTunnelFib(int fib) {
    (void)fib; // Suppress unused parameter warning
    throw std::runtime_error(
        "TUN interfaces do not support tunnel FIB operations");
  }

  // Group management methods (call base class)
  std::vector<std::string> TunInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool TunInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool TunInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  // Base class method implementations (call base class)
  int TunInterface::getMedia() const { return Interface::getMedia(); }

  bool TunInterface::setMedia(int media) { return Interface::setMedia(media); }

  int TunInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int TunInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> TunInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t TunInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool TunInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t TunInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool TunInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool TunInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  bool TunInterface::setPhysicalAddress(const std::string &address) {
    return Interface::setPhysicalAddress(address);
  }

  bool TunInterface::deletePhysicalAddress() {
    return Interface::deletePhysicalAddress();
  }

  bool TunInterface::createClone(const std::string &cloneName) {
    return Interface::createClone(cloneName);
  }

  std::vector<std::string> TunInterface::getCloners() const {
    return Interface::getCloners();
  }

  std::string TunInterface::getMacAddress() const {
    return Interface::getMacAddress();
  }

  bool TunInterface::setMacAddress(const std::string &macAddress) {
    return Interface::setMacAddress(macAddress);
  }

} // namespace libfreebsdnet::interface
