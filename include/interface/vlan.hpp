/**
 * @file interface/vlan.hpp
 * @brief VLAN interface class
 * @details C++ wrapper for VLAN network interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_VLAN_HPP
#define LIBFREEBSDNET_INTERFACE_VLAN_HPP

#include "base.hpp"
#include <string>

namespace libfreebsdnet::interface {

  /**
   * @brief VLAN interface class
   * @details Provides VLAN-specific interface operations
   */
  class VlanInterface : public Interface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "vlan0")
     * @param index Interface index
     * @param flags Interface flags
     */
    VlanInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~VlanInterface() override;

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

    /**
     * @brief Get VLAN ID
     * @return VLAN ID or -1 on error
     */
    int getVlanId() const;

    /**
     * @brief Set VLAN ID
     * @param vlanId VLAN ID (1-4094)
     * @return true on success, false on error
     */
    bool setVlanId(int vlanId);

    /**
     * @brief Get parent interface
     * @return Parent interface name or empty string on error
     */
    std::string getParentInterface() const;

    /**
     * @brief Set parent interface
     * @param parentInterface Parent interface name
     * @return true on success, false on error
     */
    bool setParentInterface(const std::string &parentInterface);

    /**
     * @brief Check if VLAN interface is valid
     * @return true if valid, false otherwise
     */
    bool isValid() const;

    // Interface base class methods
    int getFib() const override;
    bool setFib(int fib) override;
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
    int getTunnelFib() const override;
    bool setTunnelFib(int fib) override;

    bool destroy() override;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_VLAN_HPP
