/**
 * @file ethernet/frame.hpp
 * @brief Ethernet frame wrapper
 * @details Provides C++ wrapper for ethernet frame operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_ETHERNET_FRAME_HPP
#define LIBFREEBSDNET_ETHERNET_FRAME_HPP

#include <cstdint>
#include <ethernet/address.hpp>
#include <memory>
#include <vector>

namespace libfreebsdnet::ethernet {

  /**
   * @brief Ethernet frame type enumeration
   */
  enum class FrameType : uint16_t {
    IPv4 = 0x0800,
    IPv6 = 0x86DD,
    ARP = 0x0806,
    VLAN = 0x8100,
    MPLS = 0x8847,
    PPPoE_DISCOVERY = 0x8863,
    PPPoE_SESSION = 0x8864
  };

  /**
   * @brief Ethernet frame structure
   * @details Represents an ethernet frame with header and payload
   */
  class Frame {
  public:
    Frame();
    explicit Frame(const std::vector<uint8_t> &data);
    Frame(const MacAddress &destination, const MacAddress &source,
          FrameType type, const std::vector<uint8_t> &payload);
    ~Frame();

    /**
     * @brief Parse frame from raw data
     * @param data Raw frame data
     * @return true on success, false on error
     */
    bool parse(const std::vector<uint8_t> &data);

    /**
     * @brief Get destination MAC address
     * @return Destination MAC address
     */
    MacAddress getDestination() const;

    /**
     * @brief Set destination MAC address
     * @param address Destination MAC address
     */
    void setDestination(const MacAddress &address);

    /**
     * @brief Get source MAC address
     * @return Source MAC address
     */
    MacAddress getSource() const;

    /**
     * @brief Set source MAC address
     * @param address Source MAC address
     */
    void setSource(const MacAddress &address);

    /**
     * @brief Get frame type
     * @return Frame type
     */
    FrameType getType() const;

    /**
     * @brief Set frame type
     * @param type Frame type
     */
    void setType(FrameType type);

    /**
     * @brief Get frame payload
     * @return Frame payload data
     */
    std::vector<uint8_t> getPayload() const;

    /**
     * @brief Set frame payload
     * @param payload Payload data
     */
    void setPayload(const std::vector<uint8_t> &payload);

    /**
     * @brief Get frame as raw bytes
     * @return Raw frame data including header and payload
     */
    std::vector<uint8_t> toBytes() const;

    /**
     * @brief Get frame size
     * @return Total frame size in bytes
     */
    size_t getSize() const;

    /**
     * @brief Check if frame is valid
     * @return true if frame is valid, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Calculate frame checksum
     * @return Frame checksum
     */
    uint32_t calculateChecksum() const;

    /**
     * @brief Verify frame checksum
     * @return true if checksum is valid, false otherwise
     */
    bool verifyChecksum() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::ethernet

#endif // LIBFREEBSDNET_ETHERNET_FRAME_HPP
