/**
 * @file vlan/interface.hpp
 * @brief VLAN interface wrapper
 * @details Provides C++ wrapper for individual VLAN interface operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_VLAN_INTERFACE_HPP
#define LIBFREEBSDNET_VLAN_INTERFACE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vlan/manager.hpp>

namespace libfreebsdnet::vlan {

  /**
   * @brief VLAN interface class
   * @details Provides interface for managing individual VLAN interfaces
   */
  class VlanInterface {
  public:
    VlanInterface();
    explicit VlanInterface(const std::string &vlanName);
    explicit VlanInterface(const VlanConfig &config);
    ~VlanInterface();

    /**
     * @brief Create the VLAN interface
     * @param config VLAN configuration
     * @return true on success, false on error
     */
    bool create(const VlanConfig &config);

    /**
     * @brief Destroy the VLAN interface
     * @return true on success, false on error
     */
    bool destroy();

    /**
     * @brief Get VLAN interface name
     * @return VLAN interface name
     */
    std::string getName() const;

    /**
     * @brief Get VLAN configuration
     * @return Current VLAN configuration
     */
    VlanConfig getConfig() const;

    /**
     * @brief Set VLAN configuration
     * @param config New VLAN configuration
     * @return true on success, false on error
     */
    bool setConfig(const VlanConfig &config);

    /**
     * @brief Get VLAN ID
     * @return VLAN ID
     */
    uint16_t getVlanId() const;

    /**
     * @brief Set VLAN ID
     * @param vlanId New VLAN ID
     * @return true on success, false on error
     */
    bool setVlanId(uint16_t vlanId);

    /**
     * @brief Get parent interface
     * @return Parent interface name
     */
    std::string getParentInterface() const;

    /**
     * @brief Set parent interface
     * @param parentInterface New parent interface name
     * @return true on success, false on error
     */
    bool setParentInterface(const std::string &parentInterface);

    /**
     * @brief Get VLAN MTU
     * @return VLAN MTU
     */
    uint32_t getMtu() const;

    /**
     * @brief Set VLAN MTU
     * @param mtu New MTU
     * @return true on success, false on error
     */
    bool setMtu(uint32_t mtu);

    /**
     * @brief Enable or disable checksum offload
     * @param enable Enable checksum offload
     * @return true on success, false on error
     */
    bool setChecksumOffload(bool enable);

    /**
     * @brief Enable or disable TSO (TCP Segmentation Offload)
     * @param enable Enable TSO
     * @return true on success, false on error
     */
    bool setTso(bool enable);

    /**
     * @brief Enable or disable LRO (Large Receive Offload)
     * @param enable Enable LRO
     * @return true on success, false on error
     */
    bool setLro(bool enable);

    /**
     * @brief Get VLAN statistics
     * @return VLAN statistics
     */
    VlanStatistics getStatistics() const;

    /**
     * @brief Reset VLAN statistics
     * @return true on success, false on error
     */
    bool resetStatistics();

    /**
     * @brief Check if VLAN interface is up
     * @return true if interface is up, false otherwise
     */
    bool isUp() const;

    /**
     * @brief Bring VLAN interface up
     * @return true on success, false on error
     */
    bool bringUp();

    /**
     * @brief Bring VLAN interface down
     * @return true on success, false on error
     */
    bool bringDown();

    /**
     * @brief Check if VLAN interface exists
     * @return true if interface exists, false otherwise
     */
    bool exists() const;

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::vlan

#endif // LIBFREEBSDNET_VLAN_INTERFACE_HPP
