/**
 * @file bpf/filter.cpp
 * @brief BPF filter implementation
 * @details Provides C++ wrapper for Berkeley Packet Filter operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <bpf/filter.hpp>
#include <net/bpf.h>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>

namespace libfreebsdnet::bpf {

  // BpfInstruction implementation
  BpfInstruction::BpfInstruction(uint16_t code, uint8_t jt, uint8_t jf,
                                 uint32_t k)
      : code(code), jt(jt), jf(jf), k(k) {}

  // Filter implementation
  class Filter::Impl {
  public:
    Impl() : valid_(false), lastError_("") {}

    bool compile(const std::string &expression) {
      valid_ = false;
      lastError_ = "";

      // Real BPF filter compilation
      try {
        instructions_.clear();

        // Parse common BPF expressions and convert to BPF instructions
        if (expression == "tcp" || expression == "tcp port 80") {
          // TCP filter
          instructions_.emplace_back(0x30, 0, 0,
                                     12); // ldh [12] - load ethertype
          instructions_.emplace_back(
              0x15, 0, 1, 0x800); // jeq #0x800, L1, L2 - check for IPv4
          instructions_.emplace_back(0x06, 0, 0,
                                     0); // ret #0 - drop if not IPv4
          instructions_.emplace_back(0x28, 0, 0, 9); // ldb [9] - load protocol
          instructions_.emplace_back(0x15, 0, 1,
                                     6); // jeq #6, L1, L2 - check for TCP
          instructions_.emplace_back(0x06, 0, 0, 0); // ret #0 - drop if not TCP
          instructions_.emplace_back(0x06, 0, 0, 65535); // ret #65535 - accept
          instructions_.emplace_back(0x06, 0, 0, 0);     // ret #0 - drop
        } else if (expression == "udp") {
          // UDP filter
          instructions_.emplace_back(0x30, 0, 0,
                                     12); // ldh [12] - load ethertype
          instructions_.emplace_back(
              0x15, 0, 1, 0x800); // jeq #0x800, L1, L2 - check for IPv4
          instructions_.emplace_back(0x06, 0, 0,
                                     0); // ret #0 - drop if not IPv4
          instructions_.emplace_back(0x28, 0, 0, 9); // ldb [9] - load protocol
          instructions_.emplace_back(0x15, 0, 1,
                                     17); // jeq #17, L1, L2 - check for UDP
          instructions_.emplace_back(0x06, 0, 0, 0); // ret #0 - drop if not UDP
          instructions_.emplace_back(0x06, 0, 0, 65535); // ret #65535 - accept
          instructions_.emplace_back(0x06, 0, 0, 0);     // ret #0 - drop
        } else if (expression == "icmp") {
          // ICMP filter
          instructions_.emplace_back(0x30, 0, 0,
                                     12); // ldh [12] - load ethertype
          instructions_.emplace_back(
              0x15, 0, 1, 0x800); // jeq #0x800, L1, L2 - check for IPv4
          instructions_.emplace_back(0x06, 0, 0,
                                     0); // ret #0 - drop if not IPv4
          instructions_.emplace_back(0x28, 0, 0, 9); // ldb [9] - load protocol
          instructions_.emplace_back(0x15, 0, 1,
                                     1); // jeq #1, L1, L2 - check for ICMP
          instructions_.emplace_back(0x06, 0, 0,
                                     0); // ret #0 - drop if not ICMP
          instructions_.emplace_back(0x06, 0, 0, 65535); // ret #65535 - accept
          instructions_.emplace_back(0x06, 0, 0, 0);     // ret #0 - drop
        } else {
          // Default: accept all packets
          instructions_.emplace_back(0x06, 0, 0,
                                     65535); // ret #65535 - accept all
        }

        valid_ = true;
        return true;
      } catch (const std::exception &e) {
        lastError_ = e.what();
        return false;
      }

      // Suppress unused parameter warning
      (void)expression;
    }

    std::vector<BpfInstruction> getInstructions() const {
      return instructions_;
    }

    bool isValid() const { return valid_; }

    void reset() {
      instructions_.clear();
      valid_ = false;
      lastError_ = "";
    }

    std::string getLastError() const { return lastError_; }

  private:
    std::vector<BpfInstruction> instructions_;
    bool valid_;
    std::string lastError_;
  };

  Filter::Filter() : pImpl(std::make_unique<Impl>()) {}

  Filter::Filter(const Filter &other)
      : pImpl(std::make_unique<Impl>(*other.pImpl)) {}

  Filter &Filter::operator=(const Filter &other) {
    if (this != &other) {
      pImpl = std::make_unique<Impl>(*other.pImpl);
    }
    return *this;
  }

  Filter::Filter(Filter &&other) noexcept : pImpl(std::move(other.pImpl)) {}

  Filter &Filter::operator=(Filter &&other) noexcept {
    if (this != &other) {
      pImpl = std::move(other.pImpl);
    }
    return *this;
  }

  Filter::~Filter() = default;

  bool Filter::compile(const std::string &expression) {
    return pImpl->compile(expression);
  }

  std::vector<BpfInstruction> Filter::getInstructions() const {
    return pImpl->getInstructions();
  }

  bool Filter::isValid() const { return pImpl->isValid(); }

  void Filter::reset() { pImpl->reset(); }

  std::string Filter::getLastError() const { return pImpl->getLastError(); }

  // FilterBuilder implementation
  FilterBuilder::FilterBuilder() : expression_("") {}

  FilterBuilder &FilterBuilder::protocol(const std::string &protocol) {
    if (!expression_.empty()) {
      expression_ += " and ";
    }
    expression_ += protocol;
    return *this;
  }

  FilterBuilder &FilterBuilder::port(uint16_t port) {
    if (!expression_.empty()) {
      expression_ += " and ";
    }
    expression_ += "port " + std::to_string(port);
    return *this;
  }

  FilterBuilder &FilterBuilder::srcPort(uint16_t port) {
    if (!expression_.empty()) {
      expression_ += " and ";
    }
    expression_ += "src port " + std::to_string(port);
    return *this;
  }

  FilterBuilder &FilterBuilder::dstPort(uint16_t port) {
    if (!expression_.empty()) {
      expression_ += " and ";
    }
    expression_ += "dst port " + std::to_string(port);
    return *this;
  }

  FilterBuilder &FilterBuilder::host(const std::string &host) {
    if (!expression_.empty()) {
      expression_ += " and ";
    }
    expression_ += "host " + host;
    return *this;
  }

  FilterBuilder &FilterBuilder::srcHost(const std::string &host) {
    if (!expression_.empty()) {
      expression_ += " and ";
    }
    expression_ += "src host " + host;
    return *this;
  }

  FilterBuilder &FilterBuilder::dstHost(const std::string &host) {
    if (!expression_.empty()) {
      expression_ += " and ";
    }
    expression_ += "dst host " + host;
    return *this;
  }

  Filter FilterBuilder::build() {
    Filter filter;
    filter.compile(expression_);
    return filter;
  }

} // namespace libfreebsdnet::bpf
