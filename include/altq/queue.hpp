/**
 * @file altq/queue.hpp
 * @brief ALTQ queue wrapper
 * @details Provides C++ wrapper for ALTQ traffic management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_ALTQ_QUEUE_HPP
#define LIBFREEBSDNET_ALTQ_QUEUE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace libfreebsdnet::altq {

  /**
   * @brief Queue configuration parameters
   * @details Contains queue configuration settings
   */
  struct QueueConfig {
    std::string name;
    uint32_t bandwidth;
    uint32_t maxBandwidth;
    uint32_t minBandwidth;
    uint32_t weight;
    uint32_t priority;
    bool enabled;

    QueueConfig() = default;
    QueueConfig(const std::string &name, uint32_t bandwidth);
  };

  /**
   * @brief ALTQ queue interface
   * @details Provides traffic queue management functionality
   */
  class Queue {
  public:
    Queue();
    explicit Queue(const QueueConfig &config);
    ~Queue();

    /**
     * @brief Create a new queue
     * @param config Queue configuration
     * @return true on success, false on error
     */
    bool create(const QueueConfig &config);

    /**
     * @brief Delete the queue
     * @return true on success, false on error
     */
    bool destroy();

    /**
     * @brief Configure queue parameters
     * @param config New queue configuration
     * @return true on success, false on error
     */
    bool configure(const QueueConfig &config);

    /**
     * @brief Get queue statistics
     * @return Map of statistics names to values
     */
    std::unordered_map<std::string, uint64_t> getStatistics() const;

    /**
     * @brief Enable the queue
     * @return true on success, false on error
     */
    bool enable();

    /**
     * @brief Disable the queue
     * @return true on success, false on error
     */
    bool disable();

    /**
     * @brief Check if queue is enabled
     * @return true if enabled, false otherwise
     */
    bool isEnabled() const;

    /**
     * @brief Get queue configuration
     * @return Current queue configuration
     */
    QueueConfig getConfig() const;

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::altq

#endif // LIBFREEBSDNET_ALTQ_QUEUE_HPP
