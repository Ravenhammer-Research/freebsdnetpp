/**
 * @file interface/base.hpp
 * @brief Base interface class
 * @details Abstract base class for all network interface types
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_BASE_HPP
#define LIBFREEBSDNET_INTERFACE_BASE_HPP

#include <cstdint>
#include <memory>
#include <net/if.h>
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

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
    WIRELESS, // IEEE 802.11 wireless
    INFINIBAND,
    FIREWIRE,
    LAGG,           // Link Aggregation
    PFSYNC,         // PF state synchronization
    PFLOG,          // PF logging
    ENCAP,          // Encapsulation interface
    STF,            // 6to4 tunnel
    TAP,            // TAP interface
    TUN,            // TUN interface
    CARP,           // Common Address Redundancy Protocol
    L2VLAN,         // Layer 2 Virtual LAN using 802.1Q
    EPAIR,          // Ethernet pair interface
    INFINIBAND_LAG, // InfiniBand Link Aggregation
    IEEE8023AD_LAG  // IEEE 802.3ad Link Aggregation
  };

  /**
   * @brief Base interface class
   * @details Abstract base class for all network interface types
   */
  class Interface {
  public:
    virtual ~Interface() = default;

    /**
     * @brief Get interface name
     * @return Interface name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get interface index
     * @return Interface index
     */
    virtual unsigned int getIndex() const = 0;

    /**
     * @brief Get interface type
     * @return Interface type
     */
    virtual InterfaceType getType() const = 0;

    /**
     * @brief Get interface flags
     * @return Interface flags
     */
    virtual int getFlags() const = 0;

    /**
     * @brief Set interface flags
     * @param flags New flags
     * @return true on success, false on error
     */
    virtual bool setFlags(int flags) = 0;

    /**
     * @brief Bring interface up
     * @return true on success, false on error
     */
    virtual bool bringUp() = 0;

    /**
     * @brief Bring interface down
     * @return true on success, false on error
     */
    virtual bool bringDown() = 0;

    /**
     * @brief Check if interface is up
     * @return true if interface is up, false otherwise
     */
    virtual bool isUp() const = 0;

    /**
     * @brief Get interface MTU
     * @return Interface MTU or -1 on error
     */
    virtual int getMtu() const = 0;

    /**
     * @brief Set interface MTU
     * @param mtu New MTU value
     * @return true on success, false on error
     */
    virtual bool setMtu(int mtu) = 0;

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    virtual std::string getLastError() const = 0;

    /**
     * @brief Get FIB (Forwarding Information Base) assigned to this interface
     * @return FIB number or -1 if not assigned
     */
    virtual int getFib() const = 0;

    /**
     * @brief Set FIB (Forwarding Information Base) for this interface
     * @param fib FIB number to assign
     * @return true on success, false on error
     */
    virtual bool setFib(int fib) = 0;

    /**
     * @brief Get current media options for this interface
     * @return Current media options or -1 on error
     */
    virtual int getMedia() const = 0;

    /**
     * @brief Set media options for this interface
     * @param media Media options to set
     * @return true on success, false on error
     */
    virtual bool setMedia(int media) = 0;

    /**
     * @brief Get media status for this interface
     * @return Media status or -1 on error
     */
    virtual int getMediaStatus() const = 0;

    /**
     * @brief Get active media options for this interface
     * @return Active media options or -1 on error
     */
    virtual int getActiveMedia() const = 0;

    /**
     * @brief Get list of supported media options
     * @return Vector of supported media options
     */
    virtual std::vector<int> getSupportedMedia() const = 0;

    /**
     * @brief Get interface capabilities
     * @return Interface capabilities bitmask
     */
    virtual uint32_t getCapabilities() const = 0;

    /**
     * @brief Set interface capabilities
     * @param capabilities Capabilities bitmask to set
     * @return true on success, false on error
     */
    virtual bool setCapabilities(uint32_t capabilities) = 0;

    /**
     * @brief Get enabled interface capabilities
     * @return Enabled capabilities bitmask
     */
    virtual uint32_t getEnabledCapabilities() const = 0;

    /**
     * @brief Enable interface capabilities
     * @param capabilities Capabilities to enable
     * @return true on success, false on error
     */
    virtual bool enableCapabilities(uint32_t capabilities) = 0;

    /**
     * @brief Disable interface capabilities
     * @param capabilities Capabilities to disable
     * @return true on success, false on error
     */
    virtual bool disableCapabilities(uint32_t capabilities) = 0;

    /**
     * @brief Get interface groups
     * @return Vector of group names this interface belongs to
     */
    virtual std::vector<std::string> getGroups() const = 0;

    /**
     * @brief Add interface to a group
     * @param groupName Name of the group to add to
     * @return true on success, false on error
     */
    virtual bool addToGroup(const std::string &groupName) = 0;

    /**
     * @brief Remove interface from a group
     * @param groupName Name of the group to remove from
     * @return true on success, false on error
     */
    virtual bool removeFromGroup(const std::string &groupName) = 0;

    /**
     * @brief Get VNET (Virtual Network) ID for this interface
     * @return VNET ID or -1 if not assigned
     */
    virtual int getVnet() const = 0;

    /**
     * @brief Set VNET (Virtual Network) ID for this interface
     * @param vnetId VNET ID to assign
     * @return true on success, false on error
     */
    virtual bool setVnet(int vnetId) = 0;

    /**
     * @brief Reclaim interface from VNET
     * @return true on success, false on error
     */
    virtual bool reclaimFromVnet() = 0;

    /**
     * @brief Set physical address for this interface
     * @param address Physical address to set
     * @return true on success, false on error
     */
    virtual bool setPhysicalAddress(const std::string &address) = 0;

    /**
     * @brief Delete physical address from this interface
     * @return true on success, false on error
     */
    virtual bool deletePhysicalAddress() = 0;

    /**
     * @brief Create a new interface using cloning
     * @param cloneName Name for the new cloned interface
     * @return true on success, false on error
     */
    virtual bool createClone(const std::string &cloneName) = 0;

    /**
     * @brief Get list of available interface cloners
     * @return Vector of cloner names
     */
    virtual std::vector<std::string> getCloners() const = 0;

    /**
     * @brief Get MAC address for this interface
     * @return MAC address string or empty string if not available
     */
    virtual std::string getMacAddress() const = 0;

    /**
     * @brief Set MAC address for this interface
     * @param macAddress MAC address string (format: "aa:bb:cc:dd:ee:ff")
     * @return true on success, false on error
     */
    virtual bool setMacAddress(const std::string &macAddress) = 0;

    /**
     * @brief Get tunnel FIB for this interface
     * @return Tunnel FIB number or -1 if not set
     */
    virtual int getTunnelFib() const = 0;

    /**
     * @brief Set tunnel FIB for this interface
     * @param fib Tunnel FIB number
     * @return true on success, false on error
     */
    virtual bool setTunnelFib(int fib) = 0;

  protected:
    Interface() = default;
    Interface(const Interface &) = default;
    Interface &operator=(const Interface &) = default;
    Interface(Interface &&) = default;
    Interface &operator=(Interface &&) = default;
  };

  /**
   * @brief Interface factory function
   * @param name Interface name
   * @param index Interface index
   * @param flags Interface flags
   * @return Appropriate interface object or nullptr on error
   */
  std::unique_ptr<Interface> createInterface(const std::string &name,
                                             unsigned int index, int flags);

  /**
   * @brief Get interface type from name and flags
   * @param name Interface name
   * @param flags Interface flags
   * @return Interface type
   */
  InterfaceType getInterfaceType(const std::string &name, int flags);

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_BASE_HPP
