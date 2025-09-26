/**
 * @file bridge/port.cpp
 * @brief Bridge port implementation
 * @details Provides C++ wrapper for FreeBSD bridge port management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <bridge/port.hpp>

namespace libfreebsdnet::bridge {

  // BridgePort implementation
  class BridgePort::Impl {
  public:
    Impl() : config_(), lastError_("") {}

    explicit Impl(const PortConfig &config) : config_(config), lastError_("") {}

    bool configure(const PortConfig &config) {
      config_ = config;
      return true;
    }

    PortConfig getConfig() const { return config_; }

    bool setState(PortState state) {
      config_.state = state;
      return true;
    }

    PortState getState() const { return config_.state; }

    bool setLearning(bool enable) {
      config_.enableLearning = enable;
      return true;
    }

    bool setFlooding(bool enable) {
      config_.enableFlooding = enable;
      return true;
    }

    bool setSpanningTree(bool enable) {
      config_.enableSpanningTree = enable;
      return true;
    }

    PortStatistics getStatistics() const { return PortStatistics(); }

    bool resetStatistics() { return true; }

    std::string getInterfaceName() const { return config_.interfaceName; }

    std::string getLastError() const { return lastError_; }

  private:
    PortConfig config_;
    std::string lastError_;
  };

  BridgePort::BridgePort() : pImpl(std::make_unique<Impl>()) {}

  BridgePort::BridgePort(const PortConfig &config)
      : pImpl(std::make_unique<Impl>(config)) {}

  BridgePort::~BridgePort() = default;

  bool BridgePort::configure(const PortConfig &config) {
    return pImpl->configure(config);
  }

  PortConfig BridgePort::getConfig() const { return pImpl->getConfig(); }

  bool BridgePort::setState(PortState state) { return pImpl->setState(state); }

  PortState BridgePort::getState() const { return pImpl->getState(); }

  bool BridgePort::setLearning(bool enable) {
    return pImpl->setLearning(enable);
  }

  bool BridgePort::setFlooding(bool enable) {
    return pImpl->setFlooding(enable);
  }

  bool BridgePort::setSpanningTree(bool enable) {
    return pImpl->setSpanningTree(enable);
  }

  PortStatistics BridgePort::getStatistics() const {
    return pImpl->getStatistics();
  }

  bool BridgePort::resetStatistics() { return pImpl->resetStatistics(); }

  std::string BridgePort::getInterfaceName() const {
    return pImpl->getInterfaceName();
  }

  std::string BridgePort::getLastError() const { return pImpl->getLastError(); }

} // namespace libfreebsdnet::bridge
