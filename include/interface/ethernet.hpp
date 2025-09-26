/**
 * @file interface/ethernet.hpp
 * @brief Ethernet interface class
 * @details C++ wrapper for Ethernet network interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_ETHERNET_HPP
#define LIBFREEBSDNET_INTERFACE_ETHERNET_HPP

#include "base.hpp"
#include "vnet.hpp"
#include <ethernet/address.hpp>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief Ethernet interface class
   * @details Provides Ethernet-specific interface operations
   */
  class EthernetInterface : public Interface, public VnetInterface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "eth0", "em0")
     * @param index Interface index
     * @param flags Interface flags
     */
    EthernetInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~EthernetInterface() override;


    // Interface base class methods
    std::string getName() const override;
    unsigned int getIndex() const override;
    InterfaceType getType() const override;
    int getFlags() const override;
    bool setFlags(int flags) override;
    bool bringUp() override;
    bool bringDown() override;
    bool isUp() const override;
    int getMtu() const override;
    bool setMtu(int mtu) override;
    std::string getLastError() const override;
    int getFib() const override;
    bool setFib(int fib) override;

    /**
     * @brief Get MAC address
     * @return MAC address or empty address on error
     */

    /**
     * @brief Set MAC address
     * @param address New MAC address
     * @return true on success, false on error
     */
    bool setMacAddress(const libfreebsdnet::ethernet::MacAddress &address);

    /**
     * @brief Set media type
     * @param media Media type string
     * @return true on success, false on error
     */
    bool setMedia(const std::string &media);

    /**
     * @brief Check if interface supports promiscuous mode
     * @return true if supported, false otherwise
     */
    bool supportsPromiscuousMode() const;

    /**
     * @brief Enable promiscuous mode
     * @return true on success, false on error
     */
    bool enablePromiscuousMode();

    /**
     * @brief Disable promiscuous mode
     * @return true on success, false on error
     */
    bool disablePromiscuousMode();

    /**
     * @brief Check if promiscuous mode is enabled
     * @return true if enabled, false otherwise
     */
    bool isPromiscuousModeEnabled() const;

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
    int getVnet() const override;
    bool setVnet(int vnetId) override;
    bool reclaimFromVnet() override;
    bool setPhysicalAddress(const std::string &address) override;
    bool deletePhysicalAddress() override;
    bool createClone(const std::string &cloneName) override;
    std::vector<std::string> getCloners() const override;
    std::string getMacAddress() const override;
    bool setMacAddress(const std::string &macAddress) override;

    bool destroy() override;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_ETHERNET_HPP
