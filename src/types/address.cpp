/**
 * @file types/address.cpp
 * @brief Network address utilities and classes implementation
 */

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <types/address.hpp>

namespace libfreebsdnet::types {

  Address::Address(const std::string &addressString)
      : prefixLen_(0), family_(Family::UNKNOWN), valid_(false) {
    parseString(addressString);
  }

  Address::Address(const std::string &ip, int prefixLen)
      : ip_(ip), prefixLen_(prefixLen), family_(Family::UNKNOWN),
        valid_(false) {
    family_ = determineFamily(ip);
    valid_ = (family_ != Family::UNKNOWN && prefixLen_ >= 0 &&
              ((family_ == Family::IPv4 && prefixLen_ <= 32) ||
               (family_ == Family::IPv6 && prefixLen_ <= 128)));
  }

  std::string Address::getIp() const { return ip_; }

  int Address::getPrefixLength() const { return prefixLen_; }

  Address::Family Address::getFamily() const { return family_; }

  std::string Address::getNetmask() const {
    if (!valid_)
      return "";
    return prefixToNetmask(prefixLen_, family_);
  }

  std::string Address::getBroadcast() const {
    if (!valid_)
      return "";
    return calculateBroadcast(ip_, prefixLen_, family_);
  }

  std::string Address::getCidr() const {
    if (!valid_)
      return "";
    return ip_ + "/" + std::to_string(prefixLen_);
  }

  bool Address::isValid() const { return valid_; }

  bool Address::isIPv4() const { return family_ == Family::IPv4; }

  bool Address::isIPv6() const { return family_ == Family::IPv6; }

  struct sockaddr_in Address::getSockaddrIn() const {
    struct sockaddr_in addr = {};
    if (isIPv4()) {
      addr.sin_family = AF_INET;
      addr.sin_len = sizeof(addr);
      if (inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr) != 1) {
        // Set to 0.0.0.0 if conversion fails
        addr.sin_addr.s_addr = 0;
      }
    }
    return addr;
  }

  struct sockaddr_in6 Address::getSockaddrIn6() const {
    struct sockaddr_in6 addr = {};
    if (isIPv6()) {
      addr.sin6_family = AF_INET6;
      addr.sin6_len = sizeof(addr);
      if (inet_pton(AF_INET6, ip_.c_str(), &addr.sin6_addr) != 1) {
        // Set to :: if conversion fails
        std::memset(&addr.sin6_addr, 0, sizeof(addr.sin6_addr));
      }
    }
    return addr;
  }

  Address Address::fromString(const std::string &addressString) {
    return Address(addressString);
  }

  bool Address::parseAddress(const std::string &addressString, std::string &ip,
                             int &prefixLen) {
    size_t slashPos = addressString.find('/');
    if (slashPos == std::string::npos) {
      return false;
    }

    ip = addressString.substr(0, slashPos);
    std::string prefixStr = addressString.substr(slashPos + 1);

    try {
      prefixLen = std::stoi(prefixStr);
    } catch (const std::exception &) {
      return false;
    }

    return true;
  }

  std::string Address::prefixToNetmask(int prefixLen, Family family) {
    if (family == Family::IPv4) {
      if (prefixLen < 0 || prefixLen > 32)
        return "";

      uint32_t netmask = 0xFFFFFFFF << (32 - prefixLen);
      struct in_addr addr;
      addr.s_addr = htonl(netmask);

      char buf[INET_ADDRSTRLEN];
      if (inet_ntop(AF_INET, &addr, buf, INET_ADDRSTRLEN)) {
        return std::string(buf);
      }
    } else if (family == Family::IPv6) {
      if (prefixLen < 0 || prefixLen > 128)
        return "";

      // For IPv6, we'll return the prefix length as a string
      // since IPv6 netmasks are typically represented as prefix lengths
      return std::to_string(prefixLen);
    }

    return "";
  }

  std::string Address::calculateBroadcast(const std::string &ip, int prefixLen,
                                          Family family) {
    if (family == Family::IPv4) {
      struct in_addr addr;
      if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
        return "";
      }

      uint32_t netmask = 0xFFFFFFFF << (32 - prefixLen);
      uint32_t broadcast = addr.s_addr | ~netmask;

      struct in_addr broadcast_addr;
      broadcast_addr.s_addr = broadcast;

      char buf[INET_ADDRSTRLEN];
      if (inet_ntop(AF_INET, &broadcast_addr, buf, INET_ADDRSTRLEN)) {
        return std::string(buf);
      }
    }
    // IPv6 doesn't have broadcast addresses
    return "";
  }

  void Address::parseString(const std::string &addressString) {
    valid_ = false;

    if (!parseAddress(addressString, ip_, prefixLen_)) {
      return;
    }

    family_ = determineFamily(ip_);

    if (family_ == Family::UNKNOWN) {
      return;
    }

    // Validate prefix length
    if (family_ == Family::IPv4 && (prefixLen_ < 0 || prefixLen_ > 32)) {
      return;
    }
    if (family_ == Family::IPv6 && (prefixLen_ < 0 || prefixLen_ > 128)) {
      return;
    }

    valid_ = true;
  }

  Address::Family Address::determineFamily(const std::string &ip) {
    // Check for IPv4 (contains dots and no colons)
    if (ip.find('.') != std::string::npos &&
        ip.find(':') == std::string::npos) {
      struct in_addr addr;
      if (inet_pton(AF_INET, ip.c_str(), &addr) == 1) {
        return Family::IPv4;
      }
    }

    // Check for IPv6 (contains colons)
    if (ip.find(':') != std::string::npos) {
      struct in6_addr addr;
      if (inet_pton(AF_INET6, ip.c_str(), &addr) == 1) {
        return Family::IPv6;
      }
    }

    return Family::UNKNOWN;
  }

} // namespace libfreebsdnet::types
