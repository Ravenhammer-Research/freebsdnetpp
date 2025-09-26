/**
 * @file routing/table.hpp
 * @brief Routing table wrapper
 * @details Provides C++ wrapper for FreeBSD routing table operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_ROUTING_TABLE_HPP
#define LIBFREEBSDNET_ROUTING_TABLE_HPP

#include <memory>
#include <routing/entry.hpp>
#include <string>
#include <vector>

namespace libfreebsdnet::routing {

  /**
   * @brief Routing table interface
   * @details Provides access to system routing tables
   */
  class RoutingTable {
  public:
    RoutingTable();
    ~RoutingTable();

    /**
     * @brief Get all routing entries
     * @return Vector of routing entries
     */
    std::vector<std::unique_ptr<RoutingEntry>> getEntries() const;

    /**
     * @brief Get routing entries for a specific FIB
     * @param fib FIB number (0 = default FIB)
     * @return Vector of routing entries for the specified FIB
     */
    std::vector<std::unique_ptr<RoutingEntry>> getEntries(int fib) const;

    /**
     * @brief Get the number of FIBs available
     * @return Number of FIBs or -1 if not available
     */
    int getFibCount() const;

    /**
     * @brief Get the default FIB number
     * @return Default FIB number or -1 if not available
     */
    int getDefaultFib() const;

    /**
     * @brief Get routing entries for a specific destination
     * @param destination Destination address (CIDR notation)
     * @return Vector of matching routing entries
     */
    std::vector<std::unique_ptr<RoutingEntry>>
    getEntries(const std::string &destination) const;

    /**
     * @brief Add a new routing entry
     * @param destination Destination network (CIDR notation)
     * @param gateway Gateway address
     * @param interface Interface name
     * @param flags Route flags
     * @return true on success, false on error
     */
    bool addEntry(const std::string &destination, const std::string &gateway,
                  const std::string &interface, uint16_t flags = 0);

    /**
     * @brief Add a new routing entry to a specific FIB
     * @param destination Destination network (CIDR notation)
     * @param gateway Gateway address
     * @param interface Interface name
     * @param flags Route flags
     * @param fib FIB number (0 = default FIB)
     * @return true on success, false on error
     */
    bool addEntry(const std::string &destination, const std::string &gateway,
                  const std::string &interface, uint16_t flags, int fib);

    /**
     * @brief Delete a routing entry
     * @param destination Destination network (CIDR notation)
     * @param gateway Gateway address (optional, empty string for default)
     * @return true on success, false on error
     */
    bool deleteEntry(const std::string &destination,
                     const std::string &gateway = "");

    /**
     * @brief Flush all routing entries
     * @return true on success, false on error
     */
    bool flush();

    /**
     * @brief Get default gateway
     * @return Default gateway entry or nullptr if not found
     */
    std::unique_ptr<RoutingEntry> getDefaultGateway() const;

    /**
     * @brief Check if routing table is accessible
     * @return true if accessible, false otherwise
     */
    bool isAccessible() const;

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::routing

#endif // LIBFREEBSDNET_ROUTING_TABLE_HPP
