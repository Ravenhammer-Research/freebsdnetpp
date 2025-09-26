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
#include <types/address.hpp>

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
    virtual std::string getName() const;

    /**
     * @brief Get interface index
     * @return Interface index
     */
    virtual unsigned int getIndex() const;

    /**
     * @brief Get interface type
     * @return Interface type
     */
    virtual InterfaceType getType() const;

    /**
     * @brief Get interface flags
     * @return Interface flags
     */
    virtual int getFlags() const;

    /**
     * @brief Set interface flags
     * @param flags New flags
     * @return true on success, false on error
     */
    virtual bool setFlags(int flags);

    /**
     * @brief Bring interface up
     * @return true on success, false on error
     */
    virtual bool bringUp();

    /**
     * @brief Bring interface down
     * @return true on success, false on error
     */
    virtual bool bringDown();

    /**
     * @brief Check if interface is up
     * @return true if interface is up, false otherwise
     */
    virtual bool isUp() const;

    /**
     * @brief Get interface MTU
     * @return Interface MTU or -1 on error
     */
    virtual int getMtu() const;

    /**
     * @brief Set interface MTU
     * @param mtu New MTU value
     * @return true on success, false on error
     */
    virtual bool setMtu(int mtu);

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    virtual std::string getLastError() const;

    /**
     * @brief Get FIB (Forwarding Information Base) assigned to this interface
     * @return FIB number or -1 if not assigned
     */
    virtual int getFib() const;

    /**
     * @brief Set FIB (Forwarding Information Base) for this interface
     * @param fib FIB number to assign
     * @return true on success, false on error
     */
    virtual bool setFib(int fib);

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
     * @brief Get IP addresses for this interface
     * @return Vector of Address objects
     */
    virtual std::vector<libfreebsdnet::types::Address> getAddresses() const;

    /**
     * @brief Set IP address for this interface
     * @param address Address object
     * @return true on success, false on error
     */
    virtual bool setAddress(const libfreebsdnet::types::Address &address);

    /**
     * @brief Set IP address for this interface from string
     * @param addressString IP address in CIDR notation (e.g., "192.168.1.1/24")
     * @return true on success, false on error
     */
    virtual bool setAddress(const std::string &addressString);

    /**
     * @brief Set alias IP address for this interface
     * @param address Address object
     * @return true on success, false on error
     */
    virtual bool setAliasAddress(const libfreebsdnet::types::Address &address);

    /**
     * @brief Set alias IP address for this interface from string
     * @param addressString IP address in CIDR notation (e.g., "192.168.1.1/24")
     * @return true on success, false on error
     */
    virtual bool setAliasAddress(const std::string &addressString);

    /**
     * @brief Remove primary IP address from this interface
     * @return true on success, false on error
     */
    virtual bool removeAddress();

    /**
     * @brief Remove alias IP address from this interface
     * @param address Address object to remove
     * @return true on success, false on error
     */
    virtual bool removeAliasAddress(const libfreebsdnet::types::Address &address);

    /**
     * @brief Remove alias IP address from this interface
     * @param addressString IP address string to remove
     * @return true on success, false on error
     */
    virtual bool removeAliasAddress(const std::string &addressString);


    /**
     * @brief Destroy this interface
     * @return true on success, false on error
     */
    virtual bool destroy() = 0;

  protected:
    struct Impl {
      std::string name;
      unsigned int index;
      int flags;
      std::string lastError;
      
      Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags) {}
    };
    
    std::unique_ptr<Impl> pImpl;
    Interface() = default;
    Interface(const std::string &name, unsigned int index, int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}
    Interface(const Interface &) = delete;
    Interface &operator=(const Interface &) = delete;
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
