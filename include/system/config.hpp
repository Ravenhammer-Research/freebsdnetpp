/**
 * @file system/config.hpp
 * @brief System network configuration
 * @details Provides access to system network configuration via sysctl
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_SYSTEM_CONFIG_HPP
#define LIBFREEBSDNET_SYSTEM_CONFIG_HPP

#include <map>
#include <string>

namespace libfreebsdnet::system {

  /**
   * @brief System network configuration class
   * @details Provides access to system network configuration via sysctl
   */
  class SystemConfig {
  public:
    /**
     * @brief Constructor
     */
    SystemConfig();

    /**
     * @brief Destructor
     */
    ~SystemConfig();

    /**
     * @brief Get FIB configuration
     * @return Number of configured FIBs
     */
    int getFibs() const;

    /**
     * @brief Get add address to all FIBs setting
     * @return true if addresses are added to all FIBs
     */
    bool getAddAddrAllFibs() const;

    /**
     * @brief Get IPv4 forwarding status
     * @return true if IPv4 forwarding is enabled
     */
    bool getIpForwarding() const;

    /**
     * @brief Get IPv6 forwarding status
     * @return true if IPv6 forwarding is enabled
     */
    bool getIp6Forwarding() const;

    /**
     * @brief Get route multipath setting
     * @return true if multipath routing is enabled
     */
    bool getRouteMultipath() const;

    /**
     * @brief Get route hash outbound setting
     * @return true if outbound hashing is enabled
     */
    bool getRouteHashOutbound() const;

    /**
     * @brief Get IPv6 next hop setting
     * @return true if IPv6 next hop is enabled
     */
    bool getRouteIpv6Nexthop() const;

    /**
     * @brief Get IPv4 routing algorithm
     * @return IPv4 routing algorithm name
     */
    std::string getRouteInetAlgo() const;

    /**
     * @brief Get IPv6 routing algorithm
     * @return IPv6 routing algorithm name
     */
    std::string getRouteInet6Algo() const;

    /**
     * @brief Get NetISR max queue length
     * @return Maximum queue length
     */
    int getNetisrMaxqlen() const;

    /**
     * @brief Get FIB max sync delay
     * @return Maximum sync delay in milliseconds
     */
    int getFibMaxSyncDelay() const;

    /**
     * @brief Get all system configuration as a map
     * @return Map of configuration key-value pairs
     */
    std::map<std::string, std::string> getAllConfig() const;

    /**
     * @brief Get last error message
     * @return Last error message
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::system

#endif // LIBFREEBSDNET_SYSTEM_CONFIG_HPP
