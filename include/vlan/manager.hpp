/**
 * @file vlan/manager.hpp
 * @brief VLAN manager wrapper
 * @details Provides C++ wrapper for FreeBSD VLAN management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_VLAN_MANAGER_HPP
#define LIBFREEBSDNET_VLAN_MANAGER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::vlan {

  /**
   * @brief VLAN configuration structure
   * @details Contains VLAN configuration parameters
   */
  struct VlanConfig {
    std::string parentInterface;
    uint16_t vlanId;
    std::string description;
    uint32_t mtu;
    bool enableChecksumOffload;
    bool enableTso;
    bool enableLro;

    VlanConfig() = default;
    VlanConfig(const std::string &parent, uint16_t vlanId)
        : parentInterface(parent), vlanId(vlanId), mtu(1500),
          enableChecksumOffload(true), enableTso(true), enableLro(false) {}
  };

  /**
   * @brief VLAN statistics structure
   * @details Contains VLAN statistics information
   */
  struct VlanStatistics {
    uint64_t packetsReceived;
    uint64_t packetsSent;
    uint64_t bytesReceived;
    uint64_t bytesSent;
    uint64_t errorsReceived;
    uint64_t errorsSent;
    uint64_t droppedReceived;
    uint64_t droppedSent;

    VlanStatistics()
        : packetsReceived(0), packetsSent(0), bytesReceived(0), bytesSent(0),
          errorsReceived(0), errorsSent(0), droppedReceived(0), droppedSent(0) {
    }
  };

  /**
   * @brief VLAN manager class
   * @details Provides high-level interface for managing VLAN interfaces
   */
  class VlanManager {
  public:
    VlanManager();
    ~VlanManager();

    /**
     * @brief Create a new VLAN interface
     * @param config VLAN configuration
     * @return true on success, false on error
     */
    bool createVlan(const VlanConfig &config);

    /**
     * @brief Destroy a VLAN interface
     * @param vlanName VLAN interface name
     * @return true on success, false on error
     */
    bool destroyVlan(const std::string &vlanName);

    /**
     * @brief Get VLAN configuration
     * @param vlanName VLAN interface name
     * @return VLAN configuration or nullptr if not found
     */
    std::unique_ptr<VlanConfig>
    getVlanConfig(const std::string &vlanName) const;

    /**
     * @brief Set VLAN configuration
     * @param vlanName VLAN interface name
     * @param config New VLAN configuration
     * @return true on success, false on error
     */
    bool setVlanConfig(const std::string &vlanName, const VlanConfig &config);

    /**
     * @brief Get VLAN statistics
     * @param vlanName VLAN interface name
     * @return VLAN statistics or nullptr if not found
     */
    std::unique_ptr<VlanStatistics>
    getVlanStatistics(const std::string &vlanName) const;

    /**
     * @brief Get all VLANs on a parent interface
     * @param parentInterface Parent interface name
     * @return Vector of VLAN IDs
     */
    std::vector<uint16_t>
    getVlansOnInterface(const std::string &parentInterface) const;

    /**
     * @brief Get all VLAN interfaces
     * @return Vector of VLAN interface names
     */
    std::vector<std::string> getAllVlans() const;

    /**
     * @brief Check if VLAN exists
     * @param vlanName VLAN interface name
     * @return true if VLAN exists, false otherwise
     */
    bool vlanExists(const std::string &vlanName) const;

    /**
     * @brief Check if VLAN ID is in use on parent interface
     * @param parentInterface Parent interface name
     * @param vlanId VLAN ID to check
     * @return true if VLAN ID is in use, false otherwise
     */
    bool isVlanIdInUse(const std::string &parentInterface,
                       uint16_t vlanId) const;

    /**
     * @brief Get parent interface for a VLAN
     * @param vlanName VLAN interface name
     * @return Parent interface name or empty string if not found
     */
    std::string getVlanParent(const std::string &vlanName) const;

    /**
     * @brief Get VLAN ID for a VLAN interface
     * @param vlanName VLAN interface name
     * @return VLAN ID or 0 if not found
     */
    uint16_t getVlanId(const std::string &vlanName) const;

    /**
     * @brief Generate VLAN interface name
     * @param parentInterface Parent interface name
     * @param vlanId VLAN ID
     * @return Generated VLAN interface name
     */
    static std::string generateVlanName(const std::string &parentInterface,
                                        uint16_t vlanId);

    /**
     * @brief Parse VLAN interface name
     * @param vlanName VLAN interface name
     * @return Pair of (parent interface, VLAN ID) or ("", 0) if invalid
     */
    static std::pair<std::string, uint16_t>
    parseVlanName(const std::string &vlanName);

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

#endif // LIBFREEBSDNET_VLAN_MANAGER_HPP
