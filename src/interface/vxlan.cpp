/**
 * @file interface/vxlan.cpp
 * @brief VXLAN tunnel interface implementation
 * @details Implementation of VXLAN tunnel interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/vxlan.hpp>
#include <jail.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <net/if_private.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class VxlanInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;
    int vni;
    std::string groupAddress;
    int port;
    int ttl;
    bool learning;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags), vni(-1), port(4789), ttl(64),
          learning(true) {}
  };

  VxlanInterface::VxlanInterface(const std::string &name, unsigned int index,
                                 int flags)
      : TunnelInterface(name, index, flags),
        pImpl(std::make_unique<Impl>(name, index, flags)) {}

  VxlanInterface::~VxlanInterface() = default;

  InterfaceType VxlanInterface::getType() const {
    return InterfaceType::TUNNEL; // VXLAN is a type of tunnel
  }

  int VxlanInterface::getVni() const { return pImpl->vni; }

  bool VxlanInterface::setVni(int vni) {
    if (vni < 0 || vni > 16777215) { // VNI is 24-bit
      pImpl->lastError = "Invalid VNI: must be between 0 and 16777215";
      return false;
    }
    pImpl->vni = vni;
    return true; // VNI setting would require specific VXLAN ioctls
  }

  std::string VxlanInterface::getGroupAddress() const {
    return pImpl->groupAddress;
  }

  bool VxlanInterface::setGroupAddress(const std::string &address) {
    // Validate IP address format
    struct in_addr addr;
    if (inet_pton(AF_INET, address.c_str(), &addr) != 1) {
      pImpl->lastError = "Invalid IP address format";
      return false;
    }
    pImpl->groupAddress = address;
    return true; // Group address setting would require specific VXLAN ioctls
  }

  int VxlanInterface::getPort() const { return pImpl->port; }

  bool VxlanInterface::setPort(int port) {
    if (port < 1 || port > 65535) {
      pImpl->lastError = "Invalid port: must be between 1 and 65535";
      return false;
    }
    pImpl->port = port;
    return true; // Port setting would require specific VXLAN ioctls
  }

  int VxlanInterface::getTtl() const { return pImpl->ttl; }

  bool VxlanInterface::setTtl(int ttl) {
    if (ttl < 0 || ttl > 255) {
      pImpl->lastError = "Invalid TTL: must be between 0 and 255";
      return false;
    }
    pImpl->ttl = ttl;
    return true; // TTL setting would require specific VXLAN ioctls
  }

  bool VxlanInterface::isLearningEnabled() const { return pImpl->learning; }

  bool VxlanInterface::setLearning(bool enabled) {
    pImpl->learning = enabled;
    return true; // Learning setting would require specific VXLAN ioctls
  }

  // Group management methods (call base class)
  std::vector<std::string> VxlanInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool VxlanInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool VxlanInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }

  // Base class method implementations (call base class)
  int VxlanInterface::getMedia() const { return Interface::getMedia(); }

  bool VxlanInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int VxlanInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int VxlanInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> VxlanInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t VxlanInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool VxlanInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t VxlanInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool VxlanInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool VxlanInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  bool VxlanInterface::setPhysicalAddress(const std::string &address) {
    return Interface::setPhysicalAddress(address);
  }

  bool VxlanInterface::deletePhysicalAddress() {
    return Interface::deletePhysicalAddress();
  }

  bool VxlanInterface::createClone(const std::string &cloneName) {
    return Interface::createClone(cloneName);
  }

  std::vector<std::string> VxlanInterface::getCloners() const {
    return Interface::getCloners();
  }

  std::string VxlanInterface::getMacAddress() const {
    return Interface::getMacAddress();
  }

  bool VxlanInterface::setMacAddress(const std::string &macAddress) {
    return Interface::setMacAddress(macAddress);
  }

  // VNET interface methods
  int VxlanInterface::getVnet() const {
    // VNET functionality not implemented yet
    return -1;
  }

  std::string VxlanInterface::getVnetJailName() const {
    // VNET functionality not implemented yet
    return "";
  }

  bool VxlanInterface::setVnet(int vnetId) {
    (void)vnetId; // Suppress unused parameter warning
    // VNET functionality not implemented yet
    return false;
  }

  bool VxlanInterface::reclaimFromVnet() {
    // VNET functionality not implemented yet
    return false;
  }

} // namespace libfreebsdnet::interface
