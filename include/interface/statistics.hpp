/**
 * @file interface/statistics.hpp
 * @brief Network interface statistics wrapper
 * @details Provides C++ wrapper for FreeBSD network interface statistics
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_STATISTICS_HPP
#define LIBFREEBSDNET_INTERFACE_STATISTICS_HPP

#include <chrono>
#include <string>
#include <unordered_map>

namespace libfreebsdnet::interface {

  /**
   * @brief Network interface statistics structure
   * @details Contains packet and byte counters for network interfaces
   */
  struct InterfaceStatistics {
    uint64_t bytesReceived;
    uint64_t packetsReceived;
    uint64_t receiveErrors;
    uint64_t receiveDropped;
    uint64_t receiveFrameErrors;
    uint64_t receiveOverruns;

    uint64_t bytesSent;
    uint64_t packetsSent;
    uint64_t sendErrors;
    uint64_t sendDropped;
    uint64_t sendOverruns;

    uint64_t collisions;
    uint64_t carrierErrors;

    std::chrono::system_clock::time_point lastUpdated;

    InterfaceStatistics();
  };

  /**
   * @brief Interface statistics collector
   * @details Provides methods to collect and manage interface statistics
   */
  class StatisticsCollector {
  public:
    StatisticsCollector();
    ~StatisticsCollector();

    /**
     * @brief Get current statistics for an interface
     * @param interfaceName Name of the interface
     * @return Interface statistics structure
     * @throws std::runtime_error if interface not found
     */
    InterfaceStatistics getStatistics(const std::string &interfaceName) const;

    /**
     * @brief Get statistics for all interfaces
     * @return Map of interface names to their statistics
     */
    std::unordered_map<std::string, InterfaceStatistics>
    getAllStatistics() const;

    /**
     * @brief Reset statistics for an interface
     * @param interfaceName Name of the interface
     * @return true on success, false on error
     */
    bool resetStatistics(const std::string &interfaceName);

    /**
     * @brief Check if statistics collection is available
     * @return true if statistics can be collected, false otherwise
     */
    bool isAvailable() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_STATISTICS_HPP
