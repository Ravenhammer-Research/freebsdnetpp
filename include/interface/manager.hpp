/**
 * @file interface/manager.hpp
 * @brief Network interface manager wrapper
 * @details Provides C++ wrapper for FreeBSD network interface management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_MANAGER_HPP
#define LIBFREEBSDNET_INTERFACE_MANAGER_HPP

#include <interface/base.hpp>
#include <memory>
#include <net/if.h>
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief Network interface information structure
   * @details Encapsulates network interface details and statistics
   */
  struct InterfaceInfo {
    std::string name;
    unsigned int index;
    int flags;
    std::string description;
    int type;                           // Interface type (IFT_*)
    std::vector<std::string> addresses; // IP addresses
    int mtu;

    InterfaceInfo() = default;
    InterfaceInfo(const std::string &name, unsigned int index, int flags);
  };

  /**
   * @brief Network interface manager class
   * @details Provides high-level interface for managing network interfaces
   */
  class Manager {
  public:
    Manager();
    ~Manager();

    /**
     * @brief Get all available network interfaces
     * @return Vector of interface information structures
     */
    std::vector<InterfaceInfo> getInterfaces() const;

    /**
     * @brief Get interface by name
     * @param name Interface name (e.g., "eth0", "lo0")
     * @return Interface information or nullptr if not found
     */
    std::unique_ptr<InterfaceInfo> getInterface(const std::string &name) const;

    /**
     * @brief Get interface by index
     * @param index Interface index
     * @return Interface information or nullptr if not found
     */
    std::unique_ptr<InterfaceInfo> getInterface(unsigned int index) const;

    /**
     * @brief Check if interface exists
     * @param name Interface name
     * @return true if interface exists, false otherwise
     */
    bool interfaceExists(const std::string &name) const;

    /**
     * @brief Get interface flags
     * @param name Interface name
     * @return Interface flags or -1 on error
     */
    int getInterfaceFlags(const std::string &name) const;

    /**
     * @brief Get interface type enum from numeric type
     * @param type Numeric interface type
     * @return Interface type enum
     */
    static InterfaceType getInterfaceTypeFromNumeric(int type);

    /**
     * @brief Set interface flags
     * @param name Interface name
     * @param flags New interface flags
     * @return true on success, false on error
     */
    bool setInterfaceFlags(const std::string &name, int flags);

    /**
     * @brief Bring interface up
     * @param name Interface name
     * @return true on success, false on error
     */
    bool bringUp(const std::string &name);

    /**
     * @brief Bring interface down
     * @param name Interface name
     * @return true on success, false on error
     */
    bool bringDown(const std::string &name);

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_MANAGER_HPP
