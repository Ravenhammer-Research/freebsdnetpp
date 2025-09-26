/**
 * @file types/address.hpp
 * @brief Network address utilities and classes
 * @details Provides classes and utilities for handling network addresses
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_TYPES_ADDRESS_HPP
#define LIBFREEBSDNET_TYPES_ADDRESS_HPP

#include <string>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace libfreebsdnet::types {

  /**
   * @brief Network address class
   * @details Represents a network address with utilities for parsing and manipulation
   */
  class Address {
  public:
    /**
     * @brief Address family enumeration
     */
    enum class Family {
      IPv4,
      IPv6,
      UNKNOWN
    };

    /**
     * @brief Default constructor
     */
    Address() = default;

    /**
     * @brief Constructor from string
     * @param addressString Address in string format (e.g., "192.168.1.1/24")
     */
    explicit Address(const std::string &addressString);

    /**
     * @brief Constructor from IP and prefix length
     * @param ip IP address string
     * @param prefixLen Prefix length
     */
    Address(const std::string &ip, int prefixLen);

    /**
     * @brief Destructor
     */
    ~Address() = default;

    /**
     * @brief Get IP address string
     * @return IP address as string
     */
    std::string getIp() const;

    /**
     * @brief Get prefix length
     * @return Prefix length (0-32 for IPv4, 0-128 for IPv6)
     */
    int getPrefixLength() const;

    /**
     * @brief Get address family
     * @return Address family
     */
    Family getFamily() const;

    /**
     * @brief Get netmask as string
     * @return Netmask in dotted decimal notation
     */
    std::string getNetmask() const;

    /**
     * @brief Get broadcast address as string
     * @return Broadcast address
     */
    std::string getBroadcast() const;

    /**
     * @brief Get address in CIDR notation
     * @return Address in CIDR format (e.g., "192.168.1.1/24")
     */
    std::string getCidr() const;

    /**
     * @brief Check if address is valid
     * @return true if address is valid, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Check if address is IPv4
     * @return true if IPv4, false otherwise
     */
    bool isIPv4() const;

    /**
     * @brief Check if address is IPv6
     * @return true if IPv6, false otherwise
     */
    bool isIPv6() const;

    /**
     * @brief Get sockaddr_in for IPv4 addresses
     * @return sockaddr_in structure
     */
    struct sockaddr_in getSockaddrIn() const;

    /**
     * @brief Get sockaddr_in6 for IPv6 addresses
     * @return sockaddr_in6 structure
     */
    struct sockaddr_in6 getSockaddrIn6() const;

    /**
     * @brief Create address from string
     * @param addressString Address string to parse
     * @return Address object
     */
    static Address fromString(const std::string &addressString);

    /**
     * @brief Parse address string into components
     * @param addressString Address string to parse
     * @param ip Output IP address
     * @param prefixLen Output prefix length
     * @return true if parsing successful, false otherwise
     */
    static bool parseAddress(const std::string &addressString, 
                            std::string &ip, int &prefixLen);

    /**
     * @brief Convert prefix length to netmask
     * @param prefixLen Prefix length
     * @param family Address family
     * @return Netmask as string
     */
    static std::string prefixToNetmask(int prefixLen, Family family);

    /**
     * @brief Calculate broadcast address
     * @param ip IP address
     * @param prefixLen Prefix length
     * @param family Address family
     * @return Broadcast address as string
     */
    static std::string calculateBroadcast(const std::string &ip, 
                                         int prefixLen, Family family);

  private:
    std::string ip_;
    int prefixLen_;
    Family family_;
    bool valid_;

    /**
     * @brief Parse the address string
     * @param addressString Address string to parse
     */
    void parseString(const std::string &addressString);

    /**
     * @brief Determine address family from IP string
     * @param ip IP address string
     * @return Address family
     */
    static Family determineFamily(const std::string &ip);
  };

} // namespace libfreebsdnet::types

#endif // LIBFREEBSDNET_TYPES_ADDRESS_HPP
