/**
 * @file netmap/ring.hpp
 * @brief Netmap ring wrapper
 * @details Provides C++ wrapper for netmap ring operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_NETMAP_RING_HPP
#define LIBFREEBSDNET_NETMAP_RING_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace libfreebsdnet::netmap {

  /**
   * @brief Netmap slot structure
   * @details Represents a slot in a netmap ring
   */
  struct NetmapSlot {
    uint8_t *data;
    uint32_t len;
    uint16_t flags;
    uint16_t buf_idx;

    NetmapSlot() : data(nullptr), len(0), flags(0), buf_idx(0) {}
    NetmapSlot(uint8_t *data, uint32_t len, uint16_t flags, uint16_t buf_idx);
  };

  /**
   * @brief Netmap ring interface
   * @details Provides access to netmap rings for high-performance packet I/O
   */
  class NetmapRing {
  public:
    NetmapRing();
    ~NetmapRing();

    /**
     * @brief Get ring index
     * @return Ring index
     */
    uint32_t getIndex() const;

    /**
     * @brief Get number of slots in ring
     * @return Number of slots
     */
    uint32_t getNumSlots() const;

    /**
     * @brief Get current head slot index
     * @return Head slot index
     */
    uint32_t getHead() const;

    /**
     * @brief Get current tail slot index
     * @return Tail slot index
     */
    uint32_t getTail() const;

    /**
     * @brief Get current cursor slot index
     * @return Cursor slot index
     */
    uint32_t getCursor() const;

    /**
     * @brief Get slot at index
     * @param index Slot index
     * @return Netmap slot structure
     */
    NetmapSlot getSlot(uint32_t index) const;

    /**
     * @brief Get available slots for transmission
     * @return Number of available slots
     */
    uint32_t getAvailableSlots() const;

    /**
     * @brief Advance head pointer
     * @param count Number of slots to advance
     */
    void advanceHead(uint32_t count);

    /**
     * @brief Advance tail pointer
     * @param count Number of slots to advance
     */
    void advanceTail(uint32_t count);

    /**
     * @brief Check if ring is empty
     * @return true if ring is empty, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Check if ring is full
     * @return true if ring is full, false otherwise
     */
    bool isFull() const;

    /**
     * @brief Get ring statistics
     * @return Map of statistics names to values
     */
    std::unordered_map<std::string, uint64_t> getStatistics() const;

    /**
     * @brief Reset ring
     */
    void reset();

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::netmap

#endif // LIBFREEBSDNET_NETMAP_RING_HPP
