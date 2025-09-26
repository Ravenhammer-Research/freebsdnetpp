/**
 * @file routing/entry.hpp
 * @brief Routing entry wrapper
 * @details Provides C++ wrapper for FreeBSD routing entries
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_ROUTING_ENTRY_HPP
#define LIBFREEBSDNET_ROUTING_ENTRY_HPP

#include <chrono>
#include <memory>
#include <string>

namespace libfreebsdnet::routing {

  /**
   * @brief Routing entry information
   * @details Contains routing entry data and metadata
   */
  struct RoutingEntryInfo {
    std::string destination;
    std::string gateway;
    std::string interface;
    uint16_t flags;
    uint32_t metric;
    uint32_t mtu;
    std::chrono::system_clock::time_point lastUpdated;

    RoutingEntryInfo() = default;
    RoutingEntryInfo(const std::string &dest, const std::string &gw,
                     const std::string &iface, uint16_t flags, uint32_t metric);
  };

  /**
   * @brief Routing entry wrapper
   * @details Provides access to individual routing entries
   */
  class RoutingEntry {
  public:
    RoutingEntry();
    explicit RoutingEntry(const RoutingEntryInfo &info);
    ~RoutingEntry();

    /**
     * @brief Get destination network
     * @return Destination network in CIDR notation
     */
    std::string getDestination() const;

    /**
     * @brief Get gateway address
     * @return Gateway address
     */
    std::string getGateway() const;

    /**
     * @brief Get interface name
     * @return Interface name
     */
    std::string getInterface() const;

    /**
     * @brief Get route flags
     * @return Route flags
     */
    uint16_t getFlags() const;

    /**
     * @brief Get route metric
     * @return Route metric
     */
    uint32_t getMetric() const;

    /**
     * @brief Get route MTU
     * @return Route MTU
     */
    uint32_t getMtu() const;

    /**
     * @brief Check if route is active
     * @return true if route is active, false otherwise
     */
    bool isActive() const;

    /**
     * @brief Check if route is a default route
     * @return true if route is default route, false otherwise
     */
    bool isDefault() const;

    /**
     * @brief Check if route is a host route
     * @return true if route is host route, false otherwise
     */
    bool isHost() const;

    /**
     * @brief Check if route is a network route
     * @return true if route is network route, false otherwise
     */
    bool isNetwork() const;

    /**
     * @brief Get route information
     * @return Routing entry information structure
     */
    RoutingEntryInfo getInfo() const;

    /**
     * @brief Update route information
     * @param info New routing entry information
     */
    void updateInfo(const RoutingEntryInfo &info);

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::routing

#endif // LIBFREEBSDNET_ROUTING_ENTRY_HPP
