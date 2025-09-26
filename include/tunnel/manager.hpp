/**
 * @file tunnel/manager.hpp
 * @brief Tunnel manager wrapper
 * @details Provides C++ wrapper for FreeBSD tunnel management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_TUNNEL_MANAGER_HPP
#define LIBFREEBSDNET_TUNNEL_MANAGER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::tunnel {

  /**
   * @brief Tunnel type enumeration
   */
  enum class TunnelType { GRE, GIF, TAP, TUN, STF, ENCAP };

  /**
   * @brief Tunnel configuration structure
   * @details Contains tunnel configuration parameters
   */
  struct TunnelConfig {
    TunnelType type;
    std::string name;
    std::string localAddress;
    std::string remoteAddress;
    uint32_t mtu;
    uint16_t ttl;
    bool enableChecksum;
    bool enableSequence;
    bool enableKey;
    uint32_t key;

    TunnelConfig() = default;
    TunnelConfig(TunnelType type, const std::string &name)
        : type(type), name(name), mtu(1476), ttl(64), enableChecksum(false),
          enableSequence(false), enableKey(false), key(0) {}
  };

  /**
   * @brief Tunnel statistics structure
   * @details Contains tunnel statistics information
   */
  struct TunnelStatistics {
    uint64_t packetsReceived;
    uint64_t packetsSent;
    uint64_t bytesReceived;
    uint64_t bytesSent;
    uint64_t errorsReceived;
    uint64_t errorsSent;
    uint64_t droppedReceived;
    uint64_t droppedSent;

    TunnelStatistics()
        : packetsReceived(0), packetsSent(0), bytesReceived(0), bytesSent(0),
          errorsReceived(0), errorsSent(0), droppedReceived(0), droppedSent(0) {
    }
  };

  /**
   * @brief Tunnel manager class
   * @details Provides high-level interface for managing tunnel interfaces
   */
  class TunnelManager {
  public:
    TunnelManager();
    ~TunnelManager();

    /**
     * @brief Create a new tunnel interface
     * @param config Tunnel configuration
     * @return true on success, false on error
     */
    bool createTunnel(const TunnelConfig &config);

    /**
     * @brief Destroy a tunnel interface
     * @param tunnelName Tunnel interface name
     * @return true on success, false on error
     */
    bool destroyTunnel(const std::string &tunnelName);

    /**
     * @brief Get tunnel configuration
     * @param tunnelName Tunnel interface name
     * @return Tunnel configuration or nullptr if not found
     */
    std::unique_ptr<TunnelConfig>
    getTunnelConfig(const std::string &tunnelName) const;

    /**
     * @brief Set tunnel configuration
     * @param tunnelName Tunnel interface name
     * @param config New tunnel configuration
     * @return true on success, false on error
     */
    bool setTunnelConfig(const std::string &tunnelName,
                         const TunnelConfig &config);

    /**
     * @brief Get tunnel statistics
     * @param tunnelName Tunnel interface name
     * @return Tunnel statistics or nullptr if not found
     */
    std::unique_ptr<TunnelStatistics>
    getTunnelStatistics(const std::string &tunnelName) const;

    /**
     * @brief Get all tunnels of a specific type
     * @param type Tunnel type
     * @return Vector of tunnel interface names
     */
    std::vector<std::string> getTunnelsByType(TunnelType type) const;

    /**
     * @brief Get all tunnel interfaces
     * @return Vector of tunnel interface names
     */
    std::vector<std::string> getAllTunnels() const;

    /**
     * @brief Check if tunnel exists
     * @param tunnelName Tunnel interface name
     * @return true if tunnel exists, false otherwise
     */
    bool tunnelExists(const std::string &tunnelName) const;

    /**
     * @brief Get tunnel type
     * @param tunnelName Tunnel interface name
     * @return Tunnel type
     */
    TunnelType getTunnelType(const std::string &tunnelName) const;

    /**
     * @brief Set tunnel endpoints
     * @param tunnelName Tunnel interface name
     * @param localAddress Local address
     * @param remoteAddress Remote address
     * @return true on success, false on error
     */
    bool setTunnelEndpoints(const std::string &tunnelName,
                            const std::string &localAddress,
                            const std::string &remoteAddress);

    /**
     * @brief Get tunnel endpoints
     * @param tunnelName Tunnel interface name
     * @return Pair of (local address, remote address) or ("", "") if not found
     */
    std::pair<std::string, std::string>
    getTunnelEndpoints(const std::string &tunnelName) const;

    /**
     * @brief Set tunnel TTL
     * @param tunnelName Tunnel interface name
     * @param ttl TTL value
     * @return true on success, false on error
     */
    bool setTunnelTtl(const std::string &tunnelName, uint16_t ttl);

    /**
     * @brief Get tunnel TTL
     * @param tunnelName Tunnel interface name
     * @return TTL value or 0 if not found
     */
    uint16_t getTunnelTtl(const std::string &tunnelName) const;

    /**
     * @brief Set tunnel key
     * @param tunnelName Tunnel interface name
     * @param key Tunnel key
     * @return true on success, false on error
     */
    bool setTunnelKey(const std::string &tunnelName, uint32_t key);

    /**
     * @brief Get tunnel key
     * @param tunnelName Tunnel interface name
     * @return Tunnel key or 0 if not found
     */
    uint32_t getTunnelKey(const std::string &tunnelName) const;

    /**
     * @brief Enable or disable tunnel checksum
     * @param tunnelName Tunnel interface name
     * @param enable Enable checksum
     * @return true on success, false on error
     */
    bool setTunnelChecksum(const std::string &tunnelName, bool enable);

    /**
     * @brief Check if tunnel checksum is enabled
     * @param tunnelName Tunnel interface name
     * @return true if checksum is enabled, false otherwise
     */
    bool isTunnelChecksumEnabled(const std::string &tunnelName) const;

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::tunnel

#endif // LIBFREEBSDNET_TUNNEL_MANAGER_HPP
