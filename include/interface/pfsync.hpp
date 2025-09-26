/**
 * @file interface/pfsync.hpp
 * @brief PFSYNC interface class
 * @details C++ wrapper for PFSYNC network interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_PFSYNC_HPP
#define LIBFREEBSDNET_INTERFACE_PFSYNC_HPP

#include "base.hpp"
#include <string>

namespace libfreebsdnet::interface {

  /**
   * @brief PFSYNC interface class
   * @details Provides PFSYNC-specific interface operations
   */
  class PfsyncInterface : public Interface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "pfsync0")
     * @param index Interface index
     * @param flags Interface flags
     */
    PfsyncInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~PfsyncInterface() override;

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
     * @brief Get sync interface
     * @return Sync interface name or empty string on error
     */
    std::string getSyncInterface() const;

    /**
     * @brief Set sync interface
     * @param interfaceName Sync interface name
     * @return true on success, false on error
     */
    bool setSyncInterface(const std::string &interfaceName);

    /**
     * @brief Get sync peer address
     * @return Sync peer address or empty string on error
     */
    std::string getSyncPeer() const;

    /**
     * @brief Set sync peer address
     * @param peerAddress Sync peer address
     * @return true on success, false on error
     */
    bool setSyncPeer(const std::string &peerAddress);

    /**
     * @brief Get maximum updates
     * @return Maximum updates or -1 on error
     */
    int getMaxUpdates() const;

    /**
     * @brief Set maximum updates
     * @param maxUpdates Maximum updates
     * @return true on success, false on error
     */
    bool setMaxUpdates(int maxUpdates);

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

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_PFSYNC_HPP
