/**
 * @file ethernet/address.hpp
 * @brief Ethernet MAC address wrapper
 * @details Provides C++ wrapper for ethernet MAC addresses
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_ETHERNET_ADDRESS_HPP
#define LIBFREEBSDNET_ETHERNET_ADDRESS_HPP

#include <array>
#include <cstdint>
#include <string>

namespace libfreebsdnet::ethernet {

  /**
   * @brief MAC address class
   * @details Represents a 48-bit ethernet MAC address
   */
  class MacAddress {
  public:
    static constexpr size_t ADDRESS_SIZE = 6;

    MacAddress();
    explicit MacAddress(const std::array<uint8_t, ADDRESS_SIZE> &bytes);
    explicit MacAddress(const std::string &address);
    explicit MacAddress(const uint8_t *bytes);

    /**
     * @brief Get address as byte array
     * @return Array of 6 bytes representing the MAC address
     */
    std::array<uint8_t, ADDRESS_SIZE> getBytes() const;

    /**
     * @brief Get address as string
     * @return MAC address in colon-separated format (e.g., "aa:bb:cc:dd:ee:ff")
     */
    std::string toString() const;

    /**
     * @brief Get address as string with specified separator
     * @param separator Character to separate bytes
     * @return MAC address string
     */
    std::string toString(char separator) const;

    /**
     * @brief Set address from string
     * @param address MAC address string (colon, dash, or space separated)
     * @return true on success, false on error
     */
    bool fromString(const std::string &address);

    /**
     * @brief Set address from byte array
     * @param bytes Array of 6 bytes
     */
    void setBytes(const std::array<uint8_t, ADDRESS_SIZE> &bytes);

    /**
     * @brief Set address from byte pointer
     * @param bytes Pointer to 6 bytes
     */
    void setBytes(const uint8_t *bytes);

    /**
     * @brief Check if address is valid
     * @return true if address is valid, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Check if address is broadcast
     * @return true if address is broadcast (ff:ff:ff:ff:ff:ff), false otherwise
     */
    bool isBroadcast() const;

    /**
     * @brief Check if address is multicast
     * @return true if address is multicast, false otherwise
     */
    bool isMulticast() const;

    /**
     * @brief Check if address is unicast
     * @return true if address is unicast, false otherwise
     */
    bool isUnicast() const;

    /**
     * @brief Check if address is locally administered
     * @return true if address is locally administered, false otherwise
     */
    bool isLocallyAdministered() const;

    /**
     * @brief Check if address is globally administered
     * @return true if address is globally administered, false otherwise
     */
    bool isGloballyAdministered() const;

    /**
     * @brief Get OUI (Organizationally Unique Identifier)
     * @return First 3 bytes of the MAC address
     */
    std::array<uint8_t, 3> getOUI() const;

    /**
     * @brief Generate random MAC address
     * @return Random MAC address
     */
    static MacAddress random();

    /**
     * @brief Get broadcast address
     * @return Broadcast MAC address (ff:ff:ff:ff:ff:ff)
     */
    static MacAddress broadcast();

    /**
     * @brief Comparison operators
     */
    bool operator==(const MacAddress &other) const;
    bool operator!=(const MacAddress &other) const;
    bool operator<(const MacAddress &other) const;

    /**
     * @brief Array access operator
     * @param index Byte index (0-5)
     * @return Reference to byte at index
     */
    uint8_t &operator[](size_t index);
    const uint8_t &operator[](size_t index) const;

  private:
    std::array<uint8_t, ADDRESS_SIZE> bytes_;
    bool valid_;
  };

} // namespace libfreebsdnet::ethernet

#endif // LIBFREEBSDNET_ETHERNET_ADDRESS_HPP
