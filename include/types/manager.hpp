/**
 * @file types/manager.hpp
 * @brief Interface types manager wrapper
 * @details Provides C++ wrapper for FreeBSD interface types
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_TYPES_MANAGER_HPP
#define LIBFREEBSDNET_TYPES_MANAGER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::types {

  /**
   * @brief Interface type enumeration
   */
  enum class InterfaceType {
    UNKNOWN,
    ETHERNET,
    LOOPBACK,
    PPP,
    SLIP,
    TUNNEL,
    BRIDGE,
    VLAN,
    WIRELESS,
    INFINIBAND,
    FIREWIRE
  };

  /**
   * @brief Interface type information
   * @details Contains interface type information
   */
  struct InterfaceTypeInfo {
    InterfaceType type;
    std::string name;
    std::string description;
    uint32_t mtu;
    bool supportsVlans;
    bool supportsBridging;
    bool supportsTunneling;

    InterfaceTypeInfo()
        : type(InterfaceType::UNKNOWN), mtu(0), supportsVlans(false),
          supportsBridging(false), supportsTunneling(false) {}
  };

  /**
   * @brief Interface types manager class
   * @details Provides interface for managing network interface types
   */
  class InterfaceTypesManager {
  public:
    InterfaceTypesManager();
    ~InterfaceTypesManager();

    /**
     * @brief Get interface type information
     * @param interfaceName Interface name
     * @return Interface type information or nullptr if not found
     */
    std::unique_ptr<InterfaceTypeInfo>
    getInterfaceType(const std::string &interfaceName) const;

    /**
     * @brief Get interface type string
     * @param type Interface type
     * @return String representation of interface type
     */
    static std::string getInterfaceTypeString(InterfaceType type);

    /**
     * @brief Parse interface type from string
     * @param typeString Interface type string
     * @return Interface type or UNKNOWN if invalid
     */
    static InterfaceType parseInterfaceType(const std::string &typeString);

    /**
     * @brief Check if interface type supports a feature
     * @param type Interface type
     * @param feature Feature name
     * @return true if feature is supported, false otherwise
     */
    static bool supportsFeature(InterfaceType type, const std::string &feature);

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::types

#endif // LIBFREEBSDNET_TYPES_MANAGER_HPP
