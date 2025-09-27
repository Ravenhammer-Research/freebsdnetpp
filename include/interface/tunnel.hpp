/**
 * @file interface/tunnel.hpp
 * @brief Tunnel interface class
 * @details C++ wrapper for tunnel network interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_TUNNEL_HPP
#define LIBFREEBSDNET_INTERFACE_TUNNEL_HPP

#include "base.hpp"
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief Tunnel type enumeration
   */
  enum class TunnelType {
    UNKNOWN,
    GRE,   // Generic Routing Encapsulation
    GIF,   // IPv4/IPv6 tunneling
    TAP,   // TAP interface
    TUN,   // TUN interface
    IPSEC, // IPsec tunnel
    VXLAN, // VXLAN tunnel
    STF,   // 6to4 tunnel
    OVPN   // OpenVPN tunnel
  };

  /**
   * @brief Tunnel interface class
   * @details Provides tunnel-specific interface operations
   */
  class TunnelInterface : public Interface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "gif0", "tun0")
     * @param index Interface index
     * @param flags Interface flags
     */
    TunnelInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~TunnelInterface() override;

    /**
     * @brief Get local endpoint address
     * @return Local endpoint address or empty string on error
     */
    std::string getLocalEndpoint() const;

    /**
     * @brief Set local endpoint address
     * @param endpoint Local endpoint address
     * @return true on success, false on error
     */
    bool setLocalEndpoint(const std::string &endpoint);

    /**
     * @brief Get remote endpoint address
     * @return Remote endpoint address or empty string on error
     */
    std::string getRemoteEndpoint() const;

    /**
     * @brief Set remote endpoint address
     * @param endpoint Remote endpoint address
     * @return true on success, false on error
     */
    bool setRemoteEndpoint(const std::string &endpoint);

    /**
     * @brief Get tunnel key (for GRE/VXLAN)
     * @return Tunnel key or -1 on error
     */
    int getTunnelKey() const;

    /**
     * @brief Set tunnel key (for GRE/VXLAN)
     * @param key Tunnel key
     * @return true on success, false on error
     */
    bool setTunnelKey(int key);

    /**
     * @brief Check if tunnel is configured
     * @return true if configured, false otherwise
     */
    bool isConfigured() const;

    int getTunnelFib() const;
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

    bool destroy() override;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_TUNNEL_HPP
