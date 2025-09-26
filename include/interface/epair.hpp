/**
 * @file interface/epair.hpp
 * @brief Epair interface class
 * @details Implementation of epair interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_EPAIR_HPP
#define LIBFREEBSDNET_INTERFACE_EPAIR_HPP

#include <interface/base.hpp>

namespace libfreebsdnet::interface {

  /**
   * @brief Epair interface class
   * @details Represents an epair interface
   */
  class EpairInterface : public Interface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name
     * @param index Interface index
     * @param flags Interface flags
     */
    EpairInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~EpairInterface() override = default;

    /**
     * @brief Get interface type
     * @return Interface type
     */
    InterfaceType getType() const override;

    // Implement all required pure virtual methods
    int getMedia() const override;
    bool setMedia(int media) override;
    int getMediaStatus() const override;
    int getActiveMedia() const override;
    std::vector<int> getSupportedMedia() const override;
    uint32_t getCapabilities() const override;
    bool setCapabilities(uint32_t capabilities) override;
    uint32_t getEnabledCapabilities() const override;
    bool enableCapabilities(uint32_t capabilities) override;
    bool disableCapabilities(uint32_t capabilities) override;
    std::vector<std::string> getGroups() const override;
    bool addToGroup(const std::string &groupName) override;
    bool removeFromGroup(const std::string &groupName) override;
    bool setPhysicalAddress(const std::string &address) override;
    bool deletePhysicalAddress() override;
    bool createClone(const std::string &cloneName) override;
    std::vector<std::string> getCloners() const override;
    std::string getMacAddress() const override;
    bool setMacAddress(const std::string &macAddress) override;
    bool destroy() override;

  private:
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_EPAIR_HPP
