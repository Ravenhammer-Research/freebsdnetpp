/**
 * @file interface/factory.hpp
 * @brief Interface factory class
 * @details Factory for creating appropriate interface objects based on
 * interface type
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_FACTORY_HPP
#define LIBFREEBSDNET_INTERFACE_FACTORY_HPP

#include "base.hpp"
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief Interface factory class
   * @details Creates appropriate interface objects based on interface type
   */
  class InterfaceFactory {
  public:
    /**
     * @brief Create interface object based on interface name and flags
     * @param name Interface name
     * @param index Interface index
     * @param flags Interface flags
     * @return Appropriate interface object or nullptr on error
     */
    static std::unique_ptr<Interface>
    createInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Create interface object based on interface type
     * @param name Interface name
     * @param index Interface index
     * @param flags Interface flags
     * @param type Interface type
     * @return Appropriate interface object or nullptr on error
     */
    static std::unique_ptr<Interface> createInterface(const std::string &name,
                                                      unsigned int index,
                                                      int flags,
                                                      InterfaceType type);

    /**
     * @brief Get interface type from name and flags
     * @param name Interface name
     * @param flags Interface flags
     * @return Interface type
     */
    static InterfaceType getInterfaceType(const std::string &name, int flags);

    /**
     * @brief Get interface type from name only
     * @param name Interface name
     * @return Interface type
     */
    static InterfaceType getInterfaceType(const std::string &name);

    /**
     * @brief Check if interface type is supported
     * @param type Interface type
     * @return true if supported, false otherwise
     */
    static bool isSupported(InterfaceType type);

    /**
     * @brief Get supported interface types
     * @return Vector of supported interface types
     */
    static std::vector<InterfaceType> getSupportedTypes();

  private:
    /**
     * @brief Get interface type from name prefix
     * @param name Interface name
     * @return Interface type
     */
    static InterfaceType getTypeFromName(const std::string &name);

    /**
     * @brief Get interface type from flags
     * @param flags Interface flags
     * @return Interface type
     */
    static InterfaceType getTypeFromFlags(int flags);
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_FACTORY_HPP
