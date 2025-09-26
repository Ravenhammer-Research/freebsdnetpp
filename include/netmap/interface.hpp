/**
 * @file netmap/interface.hpp
 * @brief Netmap interface wrapper
 * @details Provides C++ wrapper for netmap interface operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_NETMAP_INTERFACE_HPP
#define LIBFREEBSDNET_NETMAP_INTERFACE_HPP

#include <functional>
#include <memory>
#include <netmap/ring.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace libfreebsdnet::netmap {

  /**
   * @brief Netmap interface configuration
   * @details Contains netmap interface configuration parameters
   */
  struct NetmapConfig {
    std::string interface;
    uint32_t numRxRings;
    uint32_t numTxRings;
    uint32_t bufferSize;
    bool pollMode;

    NetmapConfig() = default;
    NetmapConfig(const std::string &interface, uint32_t numRxRings = 1,
                 uint32_t numTxRings = 1);
  };

  /**
   * @brief Packet callback function type
   * @param packet Packet data
   * @param length Packet length
   * @return true to continue processing, false to stop
   */
  using PacketCallback =
      std::function<bool(const uint8_t *packet, uint32_t length)>;

  /**
   * @brief Netmap interface wrapper
   * @details Provides high-performance packet I/O using netmap
   */
  class NetmapInterface {
  public:
    NetmapInterface();
    explicit NetmapInterface(const NetmapConfig &config);
    ~NetmapInterface();

    /**
     * @brief Open netmap interface
     * @param config Netmap configuration
     * @return true on success, false on error
     */
    bool open(const NetmapConfig &config);

    /**
     * @brief Close netmap interface
     */
    void close();

    /**
     * @brief Check if interface is open
     * @return true if open, false otherwise
     */
    bool isOpen() const;

    /**
     * @brief Get interface name
     * @return Interface name
     */
    std::string getInterfaceName() const;

    /**
     * @brief Get receive ring
     * @param index Ring index
     * @return Receive ring or nullptr if not found
     */
    std::unique_ptr<NetmapRing> getRxRing(uint32_t index = 0) const;

    /**
     * @brief Get transmit ring
     * @param index Ring index
     * @return Transmit ring or nullptr if not found
     */
    std::unique_ptr<NetmapRing> getTxRing(uint32_t index = 0) const;

    /**
     * @brief Get number of receive rings
     * @return Number of receive rings
     */
    uint32_t getNumRxRings() const;

    /**
     * @brief Get number of transmit rings
     * @return Number of transmit rings
     */
    uint32_t getNumTxRings() const;

    /**
     * @brief Start packet capture
     * @param callback Function to call for each received packet
     * @return true on success, false on error
     */
    bool startCapture(PacketCallback callback);

    /**
     * @brief Stop packet capture
     */
    void stopCapture();

    /**
     * @brief Check if capture is active
     * @return true if capture is active, false otherwise
     */
    bool isCapturing() const;

    /**
     * @brief Send packet
     * @param packet Packet data
     * @param length Packet length
     * @param ringIndex Transmit ring index
     * @return true on success, false on error
     */
    bool sendPacket(const uint8_t *packet, uint32_t length,
                    uint32_t ringIndex = 0);

    /**
     * @brief Send packet from vector
     * @param packet Packet data vector
     * @param ringIndex Transmit ring index
     * @return true on success, false on error
     */
    bool sendPacket(const std::vector<uint8_t> &packet, uint32_t ringIndex = 0);

    /**
     * @brief Get interface statistics
     * @return Map of statistics names to values
     */
    std::unordered_map<std::string, uint64_t> getStatistics() const;

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::netmap

#endif // LIBFREEBSDNET_NETMAP_INTERFACE_HPP
