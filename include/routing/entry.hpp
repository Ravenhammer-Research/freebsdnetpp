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
#include <vector>

namespace libfreebsdnet::routing {

  /**
   * @brief Route flags enumeration
   * @details Individual route flags that can be combined
   */
  enum class RouteFlag {
    UP = 0x1,           // RTF_UP - Route is up
    GATEWAY = 0x2,      // RTF_GATEWAY - Route has a gateway
    HOST = 0x4,         // RTF_HOST - Host route
    REJECT = 0x8,       // RTF_REJECT - Route is rejected
    DYNAMIC = 0x10,     // RTF_DYNAMIC - Route was created dynamically
    MODIFIED = 0x20,    // RTF_MODIFIED - Route was modified dynamically
    DONE = 0x40,        // RTF_DONE - Message confirmed
    XRESOLVE = 0x200,   // RTF_XRESOLVE - External daemon resolves name
    LLINFO = 0x400,     // RTF_LLINFO - Valid protocol to link layer translation
    STATIC = 0x800,     // RTF_STATIC - Manually added route
    BLACKHOLE = 0x1000, // RTF_BLACKHOLE - Just discard packets
    PROTO2 = 0x4000,    // RTF_PROTO2 - Protocol specific routing flag
    PROTO1 = 0x8000,    // RTF_PROTO1 - Protocol specific routing flag
    PROTO3 = 0x40000,   // RTF_PROTO3 - Protocol specific routing flag
    FIXEDMTU = 0x80000, // RTF_FIXEDMTU - MTU was explicitly specified
    PINNED = 0x100000   // RTF_PINNED - Route is immutable
  };

  /**
   * @brief Routing entry information
   * @details Contains routing entry data and metadata
   */
  struct RoutingEntryInfo {
    std::string destination;
    std::string gateway;
    std::string interface;
    std::string netmask;
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
     * @return Route flags as raw value
     */
    uint16_t getFlags() const;

    /**
     * @brief Get route flags as enumeration list
     * @return Vector of individual route flags
     */
    std::vector<RouteFlag> getFlagList() const;

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
     * @brief Get route netmask
     * @return Route netmask in CIDR notation
     */
    std::string getNetmask() const;

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
