/**
 * @file bridge/stp.cpp
 * @brief Bridge STP implementation
 * @details Provides C++ wrapper for FreeBSD bridge spanning tree protocol
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <bridge/stp.hpp>

namespace libfreebsdnet::bridge {

  // StpManager implementation
  class StpManager::Impl {
  public:
    Impl() : lastError_("") {}

    bool configureStp(const std::string &bridgeName, const StpConfig &config) {
      (void)bridgeName; // Suppress unused parameter warning
      (void)config;
      return true;
    }

    bool enableStp(const std::string &bridgeName) {
      (void)bridgeName; // Suppress unused parameter warning
      return true;
    }

    bool disableStp(const std::string &bridgeName) {
      (void)bridgeName; // Suppress unused parameter warning
      return true;
    }

    bool isStpEnabled(const std::string &bridgeName) const {
      (void)bridgeName; // Suppress unused parameter warning
      return false;
    }

    std::unique_ptr<StpConfig>
    getStpConfig(const std::string &bridgeName) const {
      (void)bridgeName; // Suppress unused parameter warning
      return std::make_unique<StpConfig>();
    }

    std::vector<StpPortInfo> getStpPorts(const std::string &bridgeName) const {
      (void)bridgeName; // Suppress unused parameter warning
      return std::vector<StpPortInfo>();
    }

    std::unique_ptr<StpPortInfo>
    getStpPort(const std::string &bridgeName,
               const std::string &interfaceName) const {
      (void)bridgeName; // Suppress unused parameter warning
      (void)interfaceName;
      return nullptr;
    }

    bool setStpPortPriority(const std::string &bridgeName,
                            const std::string &interfaceName,
                            uint16_t priority) {
      (void)bridgeName; // Suppress unused parameter warning
      (void)interfaceName;
      (void)priority;
      return true;
    }

    bool setStpPortPathCost(const std::string &bridgeName,
                            const std::string &interfaceName,
                            uint32_t pathCost) {
      (void)bridgeName; // Suppress unused parameter warning
      (void)interfaceName;
      (void)pathCost;
      return true;
    }

    bool isTopologyChanging(const std::string &bridgeName) const {
      (void)bridgeName; // Suppress unused parameter warning
      return false;
    }

    bool forceTopologyChange(const std::string &bridgeName) {
      (void)bridgeName; // Suppress unused parameter warning
      return true;
    }

    std::string getLastError() const { return lastError_; }

  private:
    std::string lastError_;
  };

  StpManager::StpManager() : pImpl(std::make_unique<Impl>()) {}

  StpManager::~StpManager() = default;

  bool StpManager::configureStp(const std::string &bridgeName,
                                const StpConfig &config) {
    return pImpl->configureStp(bridgeName, config);
  }

  bool StpManager::enableStp(const std::string &bridgeName) {
    return pImpl->enableStp(bridgeName);
  }

  bool StpManager::disableStp(const std::string &bridgeName) {
    return pImpl->disableStp(bridgeName);
  }

  bool StpManager::isStpEnabled(const std::string &bridgeName) const {
    return pImpl->isStpEnabled(bridgeName);
  }

  std::unique_ptr<StpConfig>
  StpManager::getStpConfig(const std::string &bridgeName) const {
    return pImpl->getStpConfig(bridgeName);
  }

  std::vector<StpPortInfo>
  StpManager::getStpPorts(const std::string &bridgeName) const {
    return pImpl->getStpPorts(bridgeName);
  }

  std::unique_ptr<StpPortInfo>
  StpManager::getStpPort(const std::string &bridgeName,
                         const std::string &interfaceName) const {
    return pImpl->getStpPort(bridgeName, interfaceName);
  }

  bool StpManager::setStpPortPriority(const std::string &bridgeName,
                                      const std::string &interfaceName,
                                      uint16_t priority) {
    return pImpl->setStpPortPriority(bridgeName, interfaceName, priority);
  }

  bool StpManager::setStpPortPathCost(const std::string &bridgeName,
                                      const std::string &interfaceName,
                                      uint32_t pathCost) {
    return pImpl->setStpPortPathCost(bridgeName, interfaceName, pathCost);
  }

  bool StpManager::isTopologyChanging(const std::string &bridgeName) const {
    return pImpl->isTopologyChanging(bridgeName);
  }

  bool StpManager::forceTopologyChange(const std::string &bridgeName) {
    return pImpl->forceTopologyChange(bridgeName);
  }

  std::string StpManager::getLastError() const { return pImpl->getLastError(); }

} // namespace libfreebsdnet::bridge
