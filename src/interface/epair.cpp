/**
 * @file interface/epair.cpp
 * @brief Epair interface implementation
 * @details Implementation of epair interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/epair.hpp>

namespace libfreebsdnet::interface {

  EpairInterface::EpairInterface(const std::string &name, unsigned int index,
                                 int flags)
      : Interface(name, index, flags) {}

  InterfaceType EpairInterface::getType() const { return InterfaceType::EPAIR; }

  // Implement all required pure virtual methods
  int EpairInterface::getMedia() const { return Interface::getMedia(); }
  bool EpairInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }
  int EpairInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }
  int EpairInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }
  std::vector<int> EpairInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }
  uint32_t EpairInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }
  bool EpairInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }
  uint32_t EpairInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }
  bool EpairInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }
  bool EpairInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }
  std::vector<std::string> EpairInterface::getGroups() const {
    return Interface::getGroups();
  }
  bool EpairInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }
  bool EpairInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }
  bool EpairInterface::setPhysicalAddress(const std::string &address) {
    return Interface::setPhysicalAddress(address);
  }
  bool EpairInterface::deletePhysicalAddress() {
    return Interface::deletePhysicalAddress();
  }
  bool EpairInterface::createClone(const std::string &cloneName) {
    return Interface::createClone(cloneName);
  }
  std::vector<std::string> EpairInterface::getCloners() const {
    return Interface::getCloners();
  }
  std::string EpairInterface::getMacAddress() const {
    return Interface::getMacAddress();
  }
  bool EpairInterface::setMacAddress(const std::string &macAddress) {
    return Interface::setMacAddress(macAddress);
  }
  bool EpairInterface::destroy() { return Interface::destroy(); }

} // namespace libfreebsdnet::interface
