/**
 * @file interface/tap.hpp
 * @brief TAP tunnel interface class
 * @details C++ wrapper for TAP (Ethernet) tunnel interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_TAP_HPP
#define LIBFREEBSDNET_INTERFACE_TAP_HPP

#include "tunnel.hpp"

namespace libfreebsdnet::interface {

  /**
   * @brief TAP tunnel interface class
   * @details Provides TAP-specific tunnel operations
   */
  class TapInterface : public TunnelInterface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "tap0")
     * @param index Interface index
     * @param flags Interface flags
     */
    TapInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~TapInterface() override;

    /**
     * @brief Get interface type
     * @return Interface type
     */
    InterfaceType getType() const override;

    /**
     * @brief Get TAP unit number
     * @return Unit number or -1 if not set
     */
    int getUnit() const;

    /**
     * @brief Set TAP unit number
     * @param unit Unit number
     * @return true on success, false on error
     */
    bool setUnit(int unit);

    /**
     * @brief Get TAP owner
     * @return Owner UID or -1 if not set
     */
    int getOwner() const;

    /**
     * @brief Set TAP owner
     * @param uid Owner UID
     * @return true on success, false on error
     */
    bool setOwner(int uid);

    /**
     * @brief Get TAP group
     * @return Group GID or -1 if not set
     */
    int getGroup() const;

    /**
     * @brief Set TAP group
     * @param gid Group GID
     * @return true on success, false on error
     */
    bool setGroup(int gid);

    /**
     * @brief Get TAP persist flag
     * @return true if persistent, false otherwise
     */
    bool isPersistent() const;

    /**
     * @brief Set TAP persist flag
     * @param persistent Enable or disable persistence
     * @return true on success, false on error
     */
    bool setPersistent(bool persistent);

    /**
     * @brief Get TAP FIB (throws not supported error)
     * @return Always throws std::runtime_error
     */
    int getTunnelFib() const;

    /**
     * @brief Set TAP FIB (throws not supported error)
     * @param fib FIB number
     * @return Always throws std::runtime_error
     */
    bool setTunnelFib(int fib);

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

    // Group management methods (call base class)
    std::vector<std::string> getGroups() const override;
    bool addToGroup(const std::string &groupName) override;
    bool removeFromGroup(const std::string &groupName) override;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_TAP_HPP
