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
   * @brief Network interface manager class
   * @details Provides high-level interface for managing network interfaces
   */
  class Manager {
  public:
    Manager();
    ~Manager();

    /**
     * @brief Get all available network interfaces
     * @return Vector of interface objects
     */
    std::vector<std::unique_ptr<Interface>> getInterfaces() const;

    /**
     * @brief Get interface by name
     * @param name Interface name (e.g., "eth0", "lo0")
     * @return Interface object or nullptr if not found
     */
    std::unique_ptr<Interface> getInterface(const std::string &name) const;

    /**
     * @brief Get interface by index
     * @param index Interface index
     * @return Interface object or nullptr if not found
     */
    std::unique_ptr<Interface> getInterface(unsigned int index) const;

    /**
     * @brief Get interface by name with specific type
     * @tparam T Specific interface type (e.g., EthernetInterface,
     * WirelessInterface)
     * @param name Interface name
     * @return Specific interface object or nullptr if not found or wrong type
     */
    template <typename T>
    std::unique_ptr<T> getInterface(const std::string &name) const {
      auto interface = getInterface(name);
      if (!interface) {
        return nullptr;
      }

      // Try to cast to the specific type
      T *specific = dynamic_cast<T *>(interface.get());
      if (!specific) {
        return nullptr;
      }

      // Release from the generic pointer and return the specific type
      interface.release();
      return std::unique_ptr<T>(static_cast<T *>(specific));
    }

    /**
     * @brief Get interface by index with specific type
     * @tparam T Specific interface type (e.g., EthernetInterface,
     * WirelessInterface)
     * @param index Interface index
     * @return Specific interface object or nullptr if not found or wrong type
     */
    template <typename T>
    std::unique_ptr<T> getInterface(unsigned int index) const {
      auto interface = getInterface(index);
      if (!interface) {
        return nullptr;
      }

      // Try to cast to the specific type
      T *specific = dynamic_cast<T *>(interface.get());
      if (!specific) {
        return nullptr;
      }

      // Release from the generic pointer and return the specific type
      interface.release();
      return std::unique_ptr<T>(static_cast<T *>(specific));
    }

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

    /**
     * @brief Create interface object based on interface name and flags
     * @param name Interface name
     * @param index Interface index
     * @param flags Interface flags
     * @return Appropriate interface object or nullptr on error
     */
    std::unique_ptr<Interface> createInterface(const std::string &name,
                                               unsigned int index, int flags);

    /**
     * @brief Create interface object based on interface type
     * @param name Interface name
     * @param index Interface index
     * @param flags Interface flags
     * @param type Interface type
     * @return Appropriate interface object or nullptr on error
     */
    std::unique_ptr<Interface> createInterface(const std::string &name,
                                               unsigned int index, int flags,
                                               InterfaceType type);

    /**
     * @brief Check if interface type is supported
     * @param type Interface type
     * @return true if supported, false otherwise
     */
    bool isSupported(InterfaceType type);

    /**
     * @brief Get all supported interface types
     * @return Vector of supported interface types
     */
    std::vector<InterfaceType> getSupportedTypes();

  private:
    /**
     * @brief Get interface type from flags only (helper method)
     * @param flags Interface flags
     * @return Interface type
     */
    InterfaceType getTypeFromFlags(int flags);

    int socket_fd;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_MANAGER_HPP
