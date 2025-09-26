/**
 * @file bpf/capture.hpp
 * @brief BPF packet capture wrapper
 * @details Provides C++ wrapper for BPF packet capture operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_BPF_CAPTURE_HPP
#define LIBFREEBSDNET_BPF_CAPTURE_HPP

#include <bpf/filter.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace libfreebsdnet::bpf {

  /**
   * @brief Captured packet data
   * @details Contains packet data and metadata
   */
  struct CapturedPacket {
    std::vector<uint8_t> data;
    std::chrono::system_clock::time_point timestamp;
    uint32_t length;
    uint32_t caplen;
    std::string interface;

    CapturedPacket() = default;
    CapturedPacket(const std::vector<uint8_t> &data,
                   const std::chrono::system_clock::time_point &timestamp,
                   uint32_t length, uint32_t caplen,
                   const std::string &interface);
  };

  /**
   * @brief Packet capture callback function type
   * @param packet Captured packet data
   * @return true to continue capture, false to stop
   */
  using PacketCallback = std::function<bool(const CapturedPacket &)>;

  /**
   * @brief BPF packet capture interface
   * @details Provides packet capture functionality using BPF
   */
  class PacketCapture {
  public:
    PacketCapture();
    ~PacketCapture();

    /**
     * @brief Open capture interface
     * @param interfaceName Network interface name
     * @param bufferSize Buffer size for capture
     * @return true on success, false on error
     */
    bool open(const std::string &interfaceName, size_t bufferSize = 65536);

    /**
     * @brief Close capture interface
     */
    void close();

    /**
     * @brief Set capture filter
     * @param filter BPF filter to apply
     * @return true on success, false on error
     */
    bool setFilter(const Filter &filter);

    /**
     * @brief Set capture filter from expression
     * @param expression BPF filter expression
     * @return true on success, false on error
     */
    bool setFilter(const std::string &expression);

    /**
     * @brief Start packet capture
     * @param callback Function to call for each captured packet
     * @return true on success, false on error
     */
    bool startCapture(PacketCallback callback);

    /**
     * @brief Start packet capture with timeout
     * @param callback Function to call for each captured packet
     * @param timeout Timeout duration
     * @return true on success, false on error
     */
    bool startCapture(PacketCallback callback,
                      const std::chrono::milliseconds &timeout);

    /**
     * @brief Stop packet capture
     */
    void stopCapture();

    /**
     * @brief Check if capture is active
     * @return true if capture is running, false otherwise
     */
    bool isCapturing() const;

    /**
     * @brief Get capture statistics
     * @return Map of statistics names to values
     */
    std::unordered_map<std::string, uint64_t> getStatistics() const;

    /**
     * @brief Set promiscuous mode
     * @param enabled Enable or disable promiscuous mode
     * @return true on success, false on error
     */
    bool setPromiscuousMode(bool enabled);

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::bpf

#endif // LIBFREEBSDNET_BPF_CAPTURE_HPP
