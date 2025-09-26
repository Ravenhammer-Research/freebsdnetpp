/**
 * @file interface/lagg.hpp
 * @brief LAGG interface class
 * @details C++ wrapper for Link Aggregation (LAGG) network interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_LAGG_HPP
#define LIBFREEBSDNET_INTERFACE_LAGG_HPP

#include "base.hpp"
#include "vnet.hpp"
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief LAGG protocol type enumeration
   */
  enum class LagProtocol {
    UNKNOWN,
    FAILOVER,    // Failover protocol
    FEC,         // Fast EtherChannel
    LACP,        // Link Aggregation Control Protocol
    LOADBALANCE, // Load balancing
    ROUNDROBIN   // Round robin
  };

  /**
   * @brief LAGG interface class
   * @details Provides LAGG-specific interface operations
   */
  class LagInterface : public Interface, public VnetInterface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "lagg0")
     * @param index Interface index
     * @param flags Interface flags
     */
    LagInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~LagInterface() override;

    // Base class method overrides
    std::string getName() const override;
    unsigned int getIndex() const override;
    InterfaceType getType() const override;
    int getFlags() const override;
    bool setFlags(int flags) override;
    bool bringUp() override;
    bool bringDown() override;
    bool isUp() const override;
    int getMtu() const override;
    bool setMtu(int mtu) override;
    std::string getLastError() const override;
    int getFib() const override;
    bool setFib(int fib) override;

    /**
     * @brief Get LAGG protocol
     * @return LAGG protocol type
     */
    LagProtocol getProtocol() const;

    /**
     * @brief Set LAGG protocol
     * @param protocol Protocol type
     * @return true on success, false on error
     */
    bool setProtocol(LagProtocol protocol);

    /**
     * @brief Add interface to LAGG
     * @param interfaceName Interface name to add
     * @return true on success, false on error
     */
    bool addInterface(const std::string &interfaceName);

    /**
     * @brief Remove interface from LAGG
     * @param interfaceName Interface name to remove
     * @return true on success, false on error
     */
    bool removeInterface(const std::string &interfaceName);

    /**
     * @brief Get LAGG ports
     * @return Vector of port names
     */
    std::vector<std::string> getPorts() const;

    /**
     * @brief Get LAGG hash type
     * @return Hash type string (e.g., "l2,l3,l4")
     */
    std::string getHashType() const;

    /**
     * @brief Check if interface is in LAGG
     * @param interfaceName Interface name to check
     * @return true if interface is in LAGG, false otherwise
     */
    bool hasInterface(const std::string &interfaceName) const;

    /**
     * @brief Get active interface count
     * @return Number of active interfaces or -1 on error
     */
    int getActiveInterfaceCount() const;

    // Interface base class methods
    int getMedia() const override;
    bool setMedia(int media) override;
    int getMediaStatus() const override;
    int getActiveMedia() const override;
    std::vector<int> getSupportedMedia() const override;
    uint32_t getCapabilities() const override;
    bool setCapabilities(uint32_t capabilities) override;
    uint32_t getEnabledCapabilities() const override;
    bool enableCapabilities(uint32_t capabilities) override;
    bool disableCapabilities(uint32_t capabilities) override;
    std::vector<std::string> getGroups() const override;
    bool addToGroup(const std::string &groupName) override;
    bool removeFromGroup(const std::string &groupName) override;
    int getVnet() const override;
    bool setVnet(int vnetId) override;
    bool reclaimFromVnet() override;
    bool setPhysicalAddress(const std::string &address) override;
    bool deletePhysicalAddress() override;
    bool createClone(const std::string &cloneName) override;
    std::vector<std::string> getCloners() const override;
    std::string getMacAddress() const override;
    bool setMacAddress(const std::string &macAddress) override;

    bool destroy() override;

    // InfiniBand LAG-specific methods
    /**
     * @brief Check if this is an InfiniBand LAG
     * @return true if InfiniBand LAG, false otherwise
     */
    bool isInfinibandLag() const;

    /**
     * @brief Get InfiniBand address for this LAG
     * @return InfiniBand address string or empty string if not available
     */
    std::string getInfinibandAddress() const;

    /**
     * @brief Set InfiniBand address for this LAG
     * @param address InfiniBand address string
     * @return true on success, false on error
     */
    bool setInfinibandAddress(const std::string &address);

    /**
     * @brief Get InfiniBand MTU for this LAG
     * @return MTU value or -1 if not set
     */
    int getInfinibandMtu() const;

    /**
     * @brief Set InfiniBand MTU for this LAG
     * @param mtu MTU value
     * @return true on success, false on error
     */
    bool setInfinibandMtu(int mtu);

    // IEEE 802.3ad LAG-specific methods
    /**
     * @brief Check if this is an IEEE 802.3ad LAG
     * @return true if IEEE 802.3ad LAG, false otherwise
     */
    bool isIeee8023adLag() const;

    /**
     * @brief Get LACP (Link Aggregation Control Protocol) status
     * @return LACP status string
     */
    std::string getLacpStatus() const;

    /**
     * @brief Set LACP strict mode
     * @param strict true for strict mode, false for normal mode
     * @return true on success, false on error
     */
    bool setLacpStrictMode(bool strict);

    /**
     * @brief Get LACP strict mode status
     * @return true if strict mode enabled, false otherwise
     */
    bool getLacpStrictMode() const;

    /**
     * @brief Set LACP fast timeout
     * @param fast true for fast timeout, false for normal timeout
     * @return true on success, false on error
     */
    bool setLacpFastTimeout(bool fast);

    /**
     * @brief Get LACP fast timeout status
     * @return true if fast timeout enabled, false otherwise
     */
    bool getLacpFastTimeout() const;

    /**
     * @brief Get LACP partner information
     * @return Partner information string
     */
    std::string getLacpPartnerInfo() const;

    /**
     * @brief Get LACP system priority
     * @return System priority or -1 if not set
     */
    int getLacpSystemPriority() const;

    /**
     * @brief Set LACP system priority
     * @param priority System priority (0-65535)
     * @return true on success, false on error
     */
    bool setLacpSystemPriority(int priority);

  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_LAGG_HPP
