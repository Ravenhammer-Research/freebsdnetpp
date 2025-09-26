/**
 * @file altq/scheduler.hpp
 * @brief ALTQ scheduler wrapper
 * @details Provides C++ wrapper for ALTQ traffic scheduling
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_ALTQ_SCHEDULER_HPP
#define LIBFREEBSDNET_ALTQ_SCHEDULER_HPP

#include <altq/queue.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace libfreebsdnet::altq {

  /**
   * @brief Scheduler type enumeration
   */
  enum class SchedulerType { FIFO, PRIQ, CBQ, HFSC, FAIRQ, CODEL, RED, RIO };

  /**
   * @brief Scheduler configuration
   * @details Contains scheduler configuration parameters
   */
  struct SchedulerConfig {
    SchedulerType type;
    std::string interface;
    uint32_t bandwidth;
    uint32_t maxBandwidth;
    bool enabled;

    SchedulerConfig() = default;
    SchedulerConfig(SchedulerType type, const std::string &interface);
  };

  /**
   * @brief ALTQ scheduler interface
   * @details Provides traffic scheduling functionality
   */
  class Scheduler {
  public:
    Scheduler();
    explicit Scheduler(const SchedulerConfig &config);
    ~Scheduler();

    /**
     * @brief Create a new scheduler
     * @param config Scheduler configuration
     * @return true on success, false on error
     */
    bool create(const SchedulerConfig &config);

    /**
     * @brief Destroy the scheduler
     * @return true on success, false on error
     */
    bool destroy();

    /**
     * @brief Add a queue to the scheduler
     * @param queue Queue to add
     * @return true on success, false on error
     */
    bool addQueue(const Queue &queue);

    /**
     * @brief Remove a queue from the scheduler
     * @param queueName Name of queue to remove
     * @return true on success, false on error
     */
    bool removeQueue(const std::string &queueName);

    /**
     * @brief Get all queues managed by this scheduler
     * @return Vector of queue configurations
     */
    std::vector<QueueConfig> getQueues() const;

    /**
     * @brief Configure scheduler parameters
     * @param config New scheduler configuration
     * @return true on success, false on error
     */
    bool configure(const SchedulerConfig &config);

    /**
     * @brief Get scheduler statistics
     * @return Map of statistics names to values
     */
    std::unordered_map<std::string, uint64_t> getStatistics() const;

    /**
     * @brief Enable the scheduler
     * @return true on success, false on error
     */
    bool enable();

    /**
     * @brief Disable the scheduler
     * @return true on success, false on error
     */
    bool disable();

    /**
     * @brief Check if scheduler is enabled
     * @return true if enabled, false otherwise
     */
    bool isEnabled() const;

    /**
     * @brief Get scheduler configuration
     * @return Current scheduler configuration
     */
    SchedulerConfig getConfig() const;

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

#endif // LIBFREEBSDNET_ALTQ_SCHEDULER_HPP
