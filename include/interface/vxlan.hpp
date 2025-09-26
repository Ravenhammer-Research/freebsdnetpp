/**
 * @file interface/vxlan.hpp
 * @brief VXLAN tunnel interface class
 * @details C++ wrapper for VXLAN tunnel interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_VXLAN_HPP
#define LIBFREEBSDNET_INTERFACE_VXLAN_HPP

#include "tunnel.hpp"
#include "vnet.hpp"

namespace libfreebsdnet::interface {

  /**
   * @brief VXLAN tunnel interface class
   * @details Provides VXLAN-specific tunnel operations
   */
  class VxlanInterface : public TunnelInterface, public VnetInterface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "vxlan0")
     * @param index Interface index
     * @param flags Interface flags
     */
    VxlanInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~VxlanInterface() override;

    /**
     * @brief Get interface type
     * @return Interface type
     */
    InterfaceType getType() const override;

    /**
     * @brief Get VXLAN VNI (Virtual Network Identifier)
     * @return VNI or -1 if not set
     */
    int getVni() const;

    /**
     * @brief Set VXLAN VNI
     * @param vni Virtual Network Identifier
     * @return true on success, false on error
     */
    bool setVni(int vni);

    /**
     * @brief Get VXLAN group address
     * @return Group address or empty string if not set
     */
    std::string getGroupAddress() const;

    /**
     * @brief Set VXLAN group address
     * @param address Group address
     * @return true on success, false on error
     */
    bool setGroupAddress(const std::string &address);

    /**
     * @brief Get VXLAN port
     * @return Port number or -1 if not set
     */
    int getPort() const;

    /**
     * @brief Set VXLAN port
     * @param port Port number
     * @return true on success, false on error
     */
    bool setPort(int port);

    /**
     * @brief Get VXLAN TTL
     * @return TTL or -1 if not set
     */
    int getTtl() const;

    /**
     * @brief Set VXLAN TTL
     * @param ttl TTL value
     * @return true on success, false on error
     */
    bool setTtl(int ttl);

    /**
     * @brief Get VXLAN learning mode
     * @return true if learning is enabled, false otherwise
     */
    bool isLearningEnabled() const;

    /**
     * @brief Set VXLAN learning mode
     * @param enabled Enable or disable learning
     * @return true on success, false on error
     */
    bool setLearning(bool enabled);

    // Base class method overrides
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
    bool setPhysicalAddress(const std::string &address) override;
    bool deletePhysicalAddress() override;
    bool createClone(const std::string &cloneName) override;
    std::vector<std::string> getCloners() const override;
    std::string getMacAddress() const override;
    bool setMacAddress(const std::string &macAddress) override;

    // VNET interface methods
    int getVnet() const override;
    bool setVnet(int vnetId) override;
    bool reclaimFromVnet() override;

    // Group management methods (call base class)
    std::vector<std::string> getGroups() const override;
    bool addToGroup(const std::string &groupName) override;
    bool removeFromGroup(const std::string &groupName) override;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_VXLAN_HPP
