/**
 * @file interface/l2vlan.hpp
 * @brief L2VLAN interface header
 * @details Header for L2VLAN (Layer 2 Virtual LAN) interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_L2VLAN_HPP
#define LIBFREEBSDNET_INTERFACE_L2VLAN_HPP

#include <cstdint>
#include <interface/base.hpp>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief L2VLAN interface class
   * @details Provides L2VLAN (Layer 2 Virtual LAN) interface functionality
   */
  class L2VlanInterface : public Interface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name
     * @param index Interface index
     * @param flags Interface flags
     */
    L2VlanInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~L2VlanInterface() override;

    // Base class method overrides
    std::string getName() const override;
    unsigned int getIndex() const override;
    InterfaceType getType() const override;
    std::vector<Flag> getFlags() const override;
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
     * @brief Check if interface is valid
     * @return true if valid, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Set interface flags
     * @param flags New flags
     * @return true on success, false on error
     */

    /**
     * @brief Check if interface is up
     * @return true if up, false otherwise
     */

    /**
     * @brief Set interface MTU
     * @param mtu New MTU value
     * @return true on success, false on error
     */

    // Media support
    int getMedia() const override;
    bool setMedia(int media) override;
    int getMediaStatus() const override;
    int getActiveMedia() const override;
    std::vector<int> getSupportedMedia() const override;

    // Capability support
    uint32_t getCapabilities() const override;
    bool setCapabilities(uint32_t capabilities) override;
    uint32_t getEnabledCapabilities() const override;
    bool enableCapabilities(uint32_t capabilities) override;
    bool disableCapabilities(uint32_t capabilities) override;

    // Group support
    std::vector<std::string> getGroups() const override;
    bool addToGroup(const std::string &groupName) override;
    bool removeFromGroup(const std::string &groupName) override;

    // VNET support

    // Physical address support
    bool setPhysicalAddress(const std::string &address) override;
    bool deletePhysicalAddress() override;

    // Interface cloning support
    bool createClone(const std::string &cloneName) override;
    std::vector<std::string> getCloners() const override;

    // MAC address support
    std::string getMacAddress() const override;
    bool setMacAddress(const std::string &macAddress) override;

    bool destroy() override;

    // L2VLAN-specific methods
    /**
     * @brief Get trunk device for this L2VLAN
     * @return Trunk device name or empty string if not set
     */
    std::string getTrunkDevice() const;

    /**
     * @brief Set trunk device for this L2VLAN
     * @param trunkDevice Trunk device name
     * @return true on success, false on error
     */
    bool setTrunkDevice(const std::string &trunkDevice);

    /**
     * @brief Get VLAN tag for this L2VLAN
     * @return VLAN tag or -1 if not set
     */
    int getVlanTag() const;

    /**
     * @brief Set VLAN tag for this L2VLAN
     * @param tag VLAN tag (0-4095)
     * @return true on success, false on error
     */
    bool setVlanTag(int tag);

    /**
     * @brief Get PCP (Priority Code Point) for this L2VLAN
     * @return PCP value or -1 if not set
     */
    int getPcp() const;

    /**
     * @brief Set PCP (Priority Code Point) for this L2VLAN
     * @param pcp PCP value (0-7)
     * @return true on success, false on error
     */
    bool setPcp(int pcp);

    /**
     * @brief Get VLAN cookie for this L2VLAN
     * @return VLAN cookie or empty string if not set
     */
    std::string getVlanCookie() const;

    /**
     * @brief Set VLAN cookie for this L2VLAN
     * @param cookie VLAN cookie
     * @return true on success, false on error
     */
    bool setVlanCookie(const std::string &cookie);

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_L2VLAN_HPP
