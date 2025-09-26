/**
 * @file bridge/stp.hpp
 * @brief Bridge STP wrapper
 * @details Provides C++ wrapper for FreeBSD bridge spanning tree protocol
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_BRIDGE_STP_HPP
#define LIBFREEBSDNET_BRIDGE_STP_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::bridge {

  /**
   * @brief STP port role enumeration
   */
  enum class StpPortRole { ROOT, DESIGNATED, ALTERNATE, BACKUP, DISABLED };

  /**
   * @brief STP port state enumeration
   */
  enum class StpPortState {
    DISABLED,
    LISTENING,
    LEARNING,
    FORWARDING,
    BLOCKING
  };

  /**
   * @brief STP configuration structure
   * @details Contains STP configuration parameters
   */
  struct StpConfig {
    std::string bridgeId;
    uint16_t priority;
    uint32_t maxAge;
    uint32_t helloTime;
    uint32_t forwardDelay;
    uint32_t holdTime;
    bool enableRapidStp;
    bool enableMultipleSpanningTree;

    StpConfig()
        : priority(32768), maxAge(20), helloTime(2), forwardDelay(15),
          holdTime(1), enableRapidStp(false),
          enableMultipleSpanningTree(false) {}
  };

  /**
   * @brief STP port information
   * @details Contains STP port information
   */
  struct StpPortInfo {
    std::string interfaceName;
    StpPortRole role;
    StpPortState state;
    uint16_t priority;
    uint32_t pathCost;
    uint32_t designatedBridge;
    uint16_t designatedPort;
    uint32_t messageAge;
    uint32_t maxAge;
    uint32_t helloTime;
    uint32_t forwardDelay;

    StpPortInfo() = default;
  };

  /**
   * @brief STP manager class
   * @details Provides interface for managing spanning tree protocol
   */
  class StpManager {
  public:
    StpManager();
    ~StpManager();

    /**
     * @brief Configure STP on a bridge
     * @param bridgeName Bridge name
     * @param config STP configuration
     * @return true on success, false on error
     */
    bool configureStp(const std::string &bridgeName, const StpConfig &config);

    /**
     * @brief Enable STP on a bridge
     * @param bridgeName Bridge name
     * @return true on success, false on error
     */
    bool enableStp(const std::string &bridgeName);

    /**
     * @brief Disable STP on a bridge
     * @param bridgeName Bridge name
     * @return true on success, false on error
     */
    bool disableStp(const std::string &bridgeName);

    /**
     * @brief Check if STP is enabled on a bridge
     * @param bridgeName Bridge name
     * @return true if STP is enabled, false otherwise
     */
    bool isStpEnabled(const std::string &bridgeName) const;

    /**
     * @brief Get STP configuration for a bridge
     * @param bridgeName Bridge name
     * @return STP configuration or nullptr if not found
     */
    std::unique_ptr<StpConfig>
    getStpConfig(const std::string &bridgeName) const;

    /**
     * @brief Get STP port information for all ports on a bridge
     * @param bridgeName Bridge name
     * @return Vector of STP port information
     */
    std::vector<StpPortInfo> getStpPorts(const std::string &bridgeName) const;

    /**
     * @brief Get STP port information for a specific port
     * @param bridgeName Bridge name
     * @param interfaceName Interface name
     * @return STP port information or nullptr if not found
     */
    std::unique_ptr<StpPortInfo>
    getStpPort(const std::string &bridgeName,
               const std::string &interfaceName) const;

    /**
     * @brief Set STP port priority
     * @param bridgeName Bridge name
     * @param interfaceName Interface name
     * @param priority Port priority
     * @return true on success, false on error
     */
    bool setStpPortPriority(const std::string &bridgeName,
                            const std::string &interfaceName,
                            uint16_t priority);

    /**
     * @brief Set STP port path cost
     * @param bridgeName Bridge name
     * @param interfaceName Interface name
     * @param pathCost Port path cost
     * @return true on success, false on error
     */
    bool setStpPortPathCost(const std::string &bridgeName,
                            const std::string &interfaceName,
                            uint32_t pathCost);

    /**
     * @brief Get STP topology change information
     * @param bridgeName Bridge name
     * @return true if topology change is in progress, false otherwise
     */
    bool isTopologyChanging(const std::string &bridgeName) const;

    /**
     * @brief Force STP topology change
     * @param bridgeName Bridge name
     * @return true on success, false on error
     */
    bool forceTopologyChange(const std::string &bridgeName);

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::bridge

#endif // LIBFREEBSDNET_BRIDGE_STP_HPP
