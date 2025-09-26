/**
 * @file interface/gif.hpp
 * @brief GIF tunnel interface class
 * @details C++ wrapper for GIF (Generic IP-in-IP) tunnel interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_GIF_HPP
#define LIBFREEBSDNET_INTERFACE_GIF_HPP

#include "tunnel.hpp"
#include "vnet.hpp"

namespace libfreebsdnet::interface {

  /**
   * @brief GIF tunnel interface class
   * @details Provides GIF-specific tunnel operations
   */
  class GifInterface : public TunnelInterface, public VnetInterface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "gif0")
     * @param index Interface index
     * @param flags Interface flags
     */
    GifInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~GifInterface() override;

    /**
     * @brief Get interface type
     * @return Interface type
     */
    InterfaceType getType() const override;

    /**
     * @brief Get GIF protocol
     * @return Protocol number or -1 if not set
     */
    int getProtocol() const;

    /**
     * @brief Set GIF protocol
     * @param protocol Protocol number
     * @return true on success, false on error
     */
    bool setProtocol(int protocol);

    /**
     * @brief Get GIF local address
     * @return Local address or empty string if not set
     */
    std::string getLocalAddress() const;

    /**
     * @brief Set GIF local address
     * @param address Local address
     * @return true on success, false on error
     */
    bool setLocalAddress(const std::string &address);

    /**
     * @brief Get GIF remote address
     * @return Remote address or empty string if not set
     */
    std::string getRemoteAddress() const;

    /**
     * @brief Set GIF remote address
     * @param address Remote address
     * @return true on success, false on error
     */
    bool setRemoteAddress(const std::string &address);

    /**
     * @brief Get GIF TTL
     * @return TTL or -1 if not set
     */
    int getTtl() const;

    /**
     * @brief Set GIF TTL
     * @param ttl TTL value
     * @return true on success, false on error
     */
    bool setTtl(int ttl);

    /**
     * @brief Get GIF PMTU discovery status
     * @return true if PMTU discovery is enabled, false otherwise
     */
    bool isPmtuDiscoveryEnabled() const;

    /**
     * @brief Set GIF PMTU discovery
     * @param enabled Enable or disable PMTU discovery
     * @return true on success, false on error
     */
    bool setPmtuDiscovery(bool enabled);

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

#endif // LIBFREEBSDNET_INTERFACE_GIF_HPP
