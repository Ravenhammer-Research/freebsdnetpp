/**
 * @file bridge/manager.hpp
 * @brief Bridge interface manager wrapper
 * @details Provides C++ wrapper for FreeBSD bridge management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_BRIDGE_MANAGER_HPP
#define LIBFREEBSDNET_BRIDGE_MANAGER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::bridge {

  /**
   * @brief Bridge configuration structure
   * @details Contains bridge configuration parameters
   */
  struct BridgeConfig {
    std::string name;
    uint32_t maxAge;
    uint32_t helloTime;
    uint32_t forwardDelay;
    uint32_t maxAddresses;
    bool enableStp;
    bool enableLearning;
    bool enableFlooding;

    BridgeConfig() = default;
    BridgeConfig(const std::string &name)
        : name(name), maxAge(20), helloTime(2), forwardDelay(15),
          maxAddresses(1024), enableStp(true), enableLearning(true),
          enableFlooding(true) {}
  };

  /**
   * @brief Bridge statistics structure
   * @details Contains bridge statistics information
   */
  struct BridgeStatistics {
    uint64_t packetsReceived;
    uint64_t packetsSent;
    uint64_t packetsForwarded;
    uint64_t packetsDropped;
    uint64_t bytesReceived;
    uint64_t bytesSent;
    uint64_t bytesForwarded;
    uint64_t addressesLearned;

    BridgeStatistics()
        : packetsReceived(0), packetsSent(0), packetsForwarded(0),
          packetsDropped(0), bytesReceived(0), bytesSent(0), bytesForwarded(0),
          addressesLearned(0) {}
  };

  /**
   * @brief Bridge manager class
   * @details Provides high-level interface for managing bridge interfaces
   */
  class BridgeManager {
  public:
    BridgeManager();
    ~BridgeManager();

    /**
     * @brief Create a new bridge
     * @param config Bridge configuration
     * @return true on success, false on error
     */
    bool createBridge(const BridgeConfig &config);

    /**
     * @brief Destroy a bridge
     * @param bridgeName Bridge name
     * @return true on success, false on error
     */
    bool destroyBridge(const std::string &bridgeName);

    /**
     * @brief Add interface to bridge
     * @param bridgeName Bridge name
     * @param interfaceName Interface name to add
     * @return true on success, false on error
     */
    bool addInterface(const std::string &bridgeName,
                      const std::string &interfaceName);

    /**
     * @brief Remove interface from bridge
     * @param bridgeName Bridge name
     * @param interfaceName Interface name to remove
     * @return true on success, false on error
     */
    bool removeInterface(const std::string &bridgeName,
                         const std::string &interfaceName);

    /**
     * @brief Get bridge configuration
     * @param bridgeName Bridge name
     * @return Bridge configuration or nullptr if not found
     */
    std::unique_ptr<BridgeConfig>
    getBridgeConfig(const std::string &bridgeName) const;

    /**
     * @brief Set bridge configuration
     * @param bridgeName Bridge name
     * @param config New bridge configuration
     * @return true on success, false on error
     */
    bool setBridgeConfig(const std::string &bridgeName,
                         const BridgeConfig &config);

    /**
     * @brief Get bridge statistics
     * @param bridgeName Bridge name
     * @return Bridge statistics or nullptr if not found
     */
    std::unique_ptr<BridgeStatistics>
    getBridgeStatistics(const std::string &bridgeName) const;

    /**
     * @brief Get all bridge interfaces
     * @param bridgeName Bridge name
     * @return Vector of interface names
     */
    std::vector<std::string>
    getBridgeInterfaces(const std::string &bridgeName) const;

    /**
     * @brief Check if bridge exists
     * @param bridgeName Bridge name
     * @return true if bridge exists, false otherwise
     */
    bool bridgeExists(const std::string &bridgeName) const;

    /**
     * @brief Get all bridges
     * @return Vector of bridge names
     */
    std::vector<std::string> getAllBridges() const;

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

#endif // LIBFREEBSDNET_BRIDGE_MANAGER_HPP
