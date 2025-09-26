/**
 * @file bridge/port.hpp
 * @brief Bridge port wrapper
 * @details Provides C++ wrapper for FreeBSD bridge port management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_BRIDGE_PORT_HPP
#define LIBFREEBSDNET_BRIDGE_PORT_HPP

#include <cstdint>
#include <memory>
#include <string>

namespace libfreebsdnet::bridge {

  /**
   * @brief Bridge port state enumeration
   */
  enum class PortState { DISABLED, LISTENING, LEARNING, FORWARDING, BLOCKING };

  /**
   * @brief Bridge port configuration
   * @details Contains bridge port configuration parameters
   */
  struct PortConfig {
    std::string interfaceName;
    PortState state;
    uint16_t priority;
    uint32_t pathCost;
    bool enableLearning;
    bool enableFlooding;
    bool enableSpanningTree;

    PortConfig() = default;
    PortConfig(const std::string &interfaceName)
        : interfaceName(interfaceName), state(PortState::FORWARDING),
          priority(128), pathCost(100), enableLearning(true),
          enableFlooding(true), enableSpanningTree(true) {}
  };

  /**
   * @brief Bridge port statistics
   * @details Contains bridge port statistics information
   */
  struct PortStatistics {
    uint64_t packetsReceived;
    uint64_t packetsSent;
    uint64_t packetsForwarded;
    uint64_t packetsDropped;
    uint64_t bytesReceived;
    uint64_t bytesSent;
    uint64_t bytesForwarded;
    uint64_t addressesLearned;

    PortStatistics()
        : packetsReceived(0), packetsSent(0), packetsForwarded(0),
          packetsDropped(0), bytesReceived(0), bytesSent(0), bytesForwarded(0),
          addressesLearned(0) {}
  };

  /**
   * @brief Bridge port class
   * @details Provides interface for managing individual bridge ports
   */
  class BridgePort {
  public:
    BridgePort();
    explicit BridgePort(const PortConfig &config);
    ~BridgePort();

    /**
     * @brief Configure the bridge port
     * @param config Port configuration
     * @return true on success, false on error
     */
    bool configure(const PortConfig &config);

    /**
     * @brief Get port configuration
     * @return Current port configuration
     */
    PortConfig getConfig() const;

    /**
     * @brief Set port state
     * @param state New port state
     * @return true on success, false on error
     */
    bool setState(PortState state);

    /**
     * @brief Get port state
     * @return Current port state
     */
    PortState getState() const;

    /**
     * @brief Enable or disable learning on this port
     * @param enable Enable learning
     * @return true on success, false on error
     */
    bool setLearning(bool enable);

    /**
     * @brief Enable or disable flooding on this port
     * @param enable Enable flooding
     * @return true on success, false on error
     */
    bool setFlooding(bool enable);

    /**
     * @brief Enable or disable spanning tree on this port
     * @param enable Enable spanning tree
     * @return true on success, false on error
     */
    bool setSpanningTree(bool enable);

    /**
     * @brief Get port statistics
     * @return Port statistics
     */
    PortStatistics getStatistics() const;

    /**
     * @brief Reset port statistics
     * @return true on success, false on error
     */
    bool resetStatistics();

    /**
     * @brief Get interface name
     * @return Interface name
     */
    std::string getInterfaceName() const;

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

#endif // LIBFREEBSDNET_BRIDGE_PORT_HPP
