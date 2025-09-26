/**
 * @file interface/loopback.cpp
 * @brief Loopback interface implementation
 * @details Implementation of loopback interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/loopback.hpp>

namespace libfreebsdnet::interface {

  LoopbackInterface::LoopbackInterface(const std::string &name,
                                      unsigned int index, int flags)
      : Interface(name, index, flags) {}


  InterfaceType LoopbackInterface::getType() const { return InterfaceType::LOOPBACK; }

  // Implement all required pure virtual methods
  int LoopbackInterface::getMedia() const { return Interface::getMedia(); }
  bool LoopbackInterface::setMedia(int media) { return Interface::setMedia(media); }
  int LoopbackInterface::getMediaStatus() const { return Interface::getMediaStatus(); }
  int LoopbackInterface::getActiveMedia() const { return Interface::getActiveMedia(); }
  std::vector<int> LoopbackInterface::getSupportedMedia() const { return Interface::getSupportedMedia(); }
  uint32_t LoopbackInterface::getCapabilities() const { return Interface::getCapabilities(); }
  bool LoopbackInterface::setCapabilities(uint32_t capabilities) { return Interface::setCapabilities(capabilities); }
  uint32_t LoopbackInterface::getEnabledCapabilities() const { return Interface::getEnabledCapabilities(); }
  bool LoopbackInterface::enableCapabilities(uint32_t capabilities) { return Interface::enableCapabilities(capabilities); }
  bool LoopbackInterface::disableCapabilities(uint32_t capabilities) { return Interface::disableCapabilities(capabilities); }
  std::vector<std::string> LoopbackInterface::getGroups() const { return Interface::getGroups(); }
  bool LoopbackInterface::addToGroup(const std::string &groupName) { return Interface::addToGroup(groupName); }
  bool LoopbackInterface::removeFromGroup(const std::string &groupName) { return Interface::removeFromGroup(groupName); }
  bool LoopbackInterface::setPhysicalAddress(const std::string &address) { return Interface::setPhysicalAddress(address); }
  bool LoopbackInterface::deletePhysicalAddress() { return Interface::deletePhysicalAddress(); }
  bool LoopbackInterface::createClone(const std::string &cloneName) { return Interface::createClone(cloneName); }
  std::vector<std::string> LoopbackInterface::getCloners() const { return Interface::getCloners(); }
  std::string LoopbackInterface::getMacAddress() const { return Interface::getMacAddress(); }
  bool LoopbackInterface::setMacAddress(const std::string &macAddress) { return Interface::setMacAddress(macAddress); }
  bool LoopbackInterface::destroy() { return Interface::destroy(); }

} // namespace libfreebsdnet::interface
