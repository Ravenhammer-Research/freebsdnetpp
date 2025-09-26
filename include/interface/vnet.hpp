/**
 * @file interface/vnet.hpp
 * @brief VNET interface mix-in class
 * @details Mix-in class for interfaces that support VNET (Virtual Network) functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_VNET_HPP
#define LIBFREEBSDNET_INTERFACE_VNET_HPP

#include <string>

namespace libfreebsdnet::interface {

  /**
   * @brief VNET interface mix-in class
   * @details Provides VNET functionality for interfaces that support it
   */
  class VnetInterface {
  public:
    /**
     * @brief Virtual destructor
     */
    virtual ~VnetInterface() = default;

    /**
     * @brief Get VNET ID
     * @return VNET ID, or -1 if not in a VNET
     */
    virtual int getVnet() const = 0;

    /**
     * @brief Get VNET jail name
     * @return Jail name if in a VNET, empty string if not in a VNET or jail not found
     */
    virtual std::string getVnetJailName() const = 0;

    /**
     * @brief Set VNET ID
     * @param vnetId VNET ID to assign
     * @return true on success, false on failure
     */
    virtual bool setVnet(int vnetId) = 0;

    /**
     * @brief Reclaim interface from VNET
     * @return true on success, false on failure
     */
    virtual bool reclaimFromVnet() = 0;

  protected:
    /**
     * @brief Default constructor
     */
    VnetInterface() = default;

    /**
     * @brief Copy constructor
     */
    VnetInterface(const VnetInterface &) = default;

    /**
     * @brief Move constructor
     */
    VnetInterface(VnetInterface &&) = default;

    /**
     * @brief Copy assignment operator
     */
    VnetInterface &operator=(const VnetInterface &) = default;

    /**
     * @brief Move assignment operator
     */
    VnetInterface &operator=(VnetInterface &&) = default;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_VNET_HPP
