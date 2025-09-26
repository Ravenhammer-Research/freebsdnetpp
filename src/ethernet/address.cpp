/**
 * @file ethernet/address.cpp
 * @brief Ethernet MAC address implementation
 * @details Provides C++ wrapper for ethernet MAC addresses
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <algorithm>
#include <ethernet/address.hpp>
#include <iomanip>
#include <random>
#include <sstream>

namespace libfreebsdnet::ethernet {

  MacAddress::MacAddress() : valid_(false) { bytes_.fill(0); }

  MacAddress::MacAddress(const std::array<uint8_t, ADDRESS_SIZE> &bytes)
      : valid_(true) {
    bytes_ = bytes;
  }

  MacAddress::MacAddress(const std::string &address) : valid_(false) {
    fromString(address);
  }

  MacAddress::MacAddress(const uint8_t *bytes) : valid_(true) {
    std::copy(bytes, bytes + ADDRESS_SIZE, bytes_.begin());
  }

  std::array<uint8_t, MacAddress::ADDRESS_SIZE> MacAddress::getBytes() const {
    return bytes_;
  }

  std::string MacAddress::toString() const { return toString(':'); }

  std::string MacAddress::toString(char separator) const {
    std::ostringstream oss;
    for (size_t i = 0; i < ADDRESS_SIZE; ++i) {
      if (i > 0) {
        oss << separator;
      }
      oss << std::hex << std::setw(2) << std::setfill('0')
          << static_cast<int>(bytes_[i]);
    }
    return oss.str();
  }

  bool MacAddress::fromString(const std::string &address) {
    std::string cleanAddress = address;

    // Remove common separators
    std::replace(cleanAddress.begin(), cleanAddress.end(), ':', ' ');
    std::replace(cleanAddress.begin(), cleanAddress.end(), '-', ' ');

    std::istringstream iss(cleanAddress);
    std::string byteStr;
    size_t index = 0;

    while (iss >> byteStr && index < ADDRESS_SIZE) {
      if (byteStr.length() == 2) {
        try {
          bytes_[index] =
              static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
          ++index;
        } catch (const std::exception &) {
          valid_ = false;
          return false;
        }
      } else {
        valid_ = false;
        return false;
      }
    }

    valid_ = (index == ADDRESS_SIZE);
    return valid_;
  }

  void MacAddress::setBytes(const std::array<uint8_t, ADDRESS_SIZE> &bytes) {
    bytes_ = bytes;
    valid_ = true;
  }

  void MacAddress::setBytes(const uint8_t *bytes) {
    std::copy(bytes, bytes + ADDRESS_SIZE, bytes_.begin());
    valid_ = true;
  }

  bool MacAddress::isValid() const { return valid_; }

  bool MacAddress::isBroadcast() const {
    return std::all_of(bytes_.begin(), bytes_.end(),
                       [](uint8_t b) { return b == 0xFF; });
  }

  bool MacAddress::isMulticast() const { return (bytes_[0] & 0x01) != 0; }

  bool MacAddress::isUnicast() const {
    return !isBroadcast() && !isMulticast();
  }

  bool MacAddress::isLocallyAdministered() const {
    return (bytes_[0] & 0x02) != 0;
  }

  bool MacAddress::isGloballyAdministered() const {
    return !isLocallyAdministered();
  }

  std::array<uint8_t, 3> MacAddress::getOUI() const {
    std::array<uint8_t, 3> oui;
    std::copy(bytes_.begin(), bytes_.begin() + 3, oui.begin());
    return oui;
  }

  MacAddress MacAddress::random() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint8_t> dis(0, 255);

    MacAddress addr;
    for (size_t i = 0; i < ADDRESS_SIZE; ++i) {
      addr.bytes_[i] = dis(gen);
    }

    // Ensure it's locally administered and unicast
    addr.bytes_[0] |= 0x02;  // Set locally administered bit
    addr.bytes_[0] &= ~0x01; // Clear multicast bit

    addr.valid_ = true;
    return addr;
  }

  MacAddress MacAddress::broadcast() {
    MacAddress addr;
    addr.bytes_.fill(0xFF);
    addr.valid_ = true;
    return addr;
  }

  bool MacAddress::operator==(const MacAddress &other) const {
    return bytes_ == other.bytes_;
  }

  bool MacAddress::operator!=(const MacAddress &other) const {
    return !(*this == other);
  }

  bool MacAddress::operator<(const MacAddress &other) const {
    return bytes_ < other.bytes_;
  }

  uint8_t &MacAddress::operator[](size_t index) { return bytes_[index]; }

  const uint8_t &MacAddress::operator[](size_t index) const {
    return bytes_[index];
  }

} // namespace libfreebsdnet::ethernet
