/**
 * @file bpf/filter.hpp
 * @brief BPF filter wrapper
 * @details Provides C++ wrapper for Berkeley Packet Filter operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_BPF_FILTER_HPP
#define LIBFREEBSDNET_BPF_FILTER_HPP

#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::bpf {

  /**
   * @brief BPF instruction structure
   * @details Represents a single BPF instruction
   */
  struct BpfInstruction {
    uint16_t code;
    uint8_t jt;
    uint8_t jf;
    uint32_t k;

    BpfInstruction() = default;
    BpfInstruction(uint16_t code, uint8_t jt, uint8_t jf, uint32_t k);
  };

  /**
   * @brief BPF filter program
   * @details Manages BPF filter programs and compilation
   */
  class Filter {
  public:
    Filter();
    Filter(const Filter &other);
    Filter &operator=(const Filter &other);
    Filter(Filter &&other) noexcept;
    Filter &operator=(Filter &&other) noexcept;
    ~Filter();

    /**
     * @brief Compile filter expression
     * @param expression BPF filter expression (tcpdump syntax)
     * @return true on success, false on error
     */
    bool compile(const std::string &expression);

    /**
     * @brief Get compiled filter instructions
     * @return Vector of BPF instructions
     */
    std::vector<BpfInstruction> getInstructions() const;

    /**
     * @brief Check if filter is valid
     * @return true if filter is compiled and valid
     */
    bool isValid() const;

    /**
     * @brief Reset filter to empty state
     */
    void reset();

    /**
     * @brief Get last compilation error
     * @return Error message from last compilation attempt
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

  /**
   * @brief BPF filter builder
   * @details Provides convenient methods to build BPF filters programmatically
   */
  class FilterBuilder {
  public:
    FilterBuilder();

    /**
     * @brief Add protocol filter (e.g., "ip", "tcp", "udp")
     * @param protocol Protocol name
     * @return Reference to builder for chaining
     */
    FilterBuilder &protocol(const std::string &protocol);

    /**
     * @brief Add port filter
     * @param port Port number
     * @return Reference to builder for chaining
     */
    FilterBuilder &port(uint16_t port);

    /**
     * @brief Add source port filter
     * @param port Source port number
     * @return Reference to builder for chaining
     */
    FilterBuilder &srcPort(uint16_t port);

    /**
     * @brief Add destination port filter
     * @param port Destination port number
     * @return Reference to builder for chaining
     */
    FilterBuilder &dstPort(uint16_t port);

    /**
     * @brief Add host filter
     * @param host Host address (IP or hostname)
     * @return Reference to builder for chaining
     */
    FilterBuilder &host(const std::string &host);

    /**
     * @brief Add source host filter
     * @param host Source host address
     * @return Reference to builder for chaining
     */
    FilterBuilder &srcHost(const std::string &host);

    /**
     * @brief Add destination host filter
     * @param host Destination host address
     * @return Reference to builder for chaining
     */
    FilterBuilder &dstHost(const std::string &host);

    /**
     * @brief Build the filter
     * @return Compiled BPF filter
     */
    Filter build();

  private:
    std::string expression_;
  };

} // namespace libfreebsdnet::bpf

#endif // LIBFREEBSDNET_BPF_FILTER_HPP
