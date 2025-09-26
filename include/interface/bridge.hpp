/**
 * @file interface/bridge.hpp
 * @brief Bridge interface class
 * @details C++ wrapper for bridge network interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_BRIDGE_HPP
#define LIBFREEBSDNET_INTERFACE_BRIDGE_HPP

#include "base.hpp"
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief Bridge interface class
   * @details Provides bridge-specific interface operations
   */
  class BridgeInterface : public Interface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "bridge0")
     * @param index Interface index
     * @param flags Interface flags
     */
    BridgeInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~BridgeInterface() override;

    // Interface base class methods
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

    /**
     * @brief Add interface to bridge
     * @param interfaceName Interface name to add
     * @return true on success, false on error
     */
    bool addInterface(const std::string &interfaceName);

    /**
     * @brief Remove interface from bridge
     * @param interfaceName Interface name to remove
     * @return true on success, false on error
     */
    bool removeInterface(const std::string &interfaceName);

    /**
     * @brief Get interfaces in bridge
     * @return Vector of interface names
     */
    std::vector<std::string> getInterfaces() const;

    /**
     * @brief Check if interface is in bridge
     * @param interfaceName Interface name to check
     * @return true if interface is in bridge, false otherwise
     */
    bool hasInterface(const std::string &interfaceName) const;

    /**
     * @brief Enable spanning tree protocol
     * @return true on success, false on error
     */
    bool enableStp();

    /**
     * @brief Disable spanning tree protocol
     * @return true on success, false on error
     */
    bool disableStp();

    /**
     * @brief Check if STP is enabled
     * @return true if STP is enabled, false otherwise
     */
    bool isStpEnabled() const;

    /**
     * @brief Set bridge priority
     * @param priority Bridge priority (0-65535)
     * @return true on success, false on error
     */
    bool setPriority(uint16_t priority);

    /**
     * @brief Get bridge priority
     * @return Bridge priority or -1 on error
     */
    int getPriority() const;

    /**
     * @brief Set bridge aging time
     * @param seconds Aging time in seconds
     * @return true on success, false on error
     */
    bool setAgingTime(int seconds);

    /**
     * @brief Get bridge aging time
     * @return Aging time in seconds or -1 on error
     */
    int getAgingTime() const;

    // Interface base class methods
    int getFib() const override;
    bool setFib(int fib) override;
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
    int getTunnelFib() const override;
    bool setTunnelFib(int fib) override;

    bool destroy() override;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_BRIDGE_HPP
