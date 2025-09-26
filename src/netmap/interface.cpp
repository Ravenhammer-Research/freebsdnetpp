/**
 * @file netmap/interface.cpp
 * @brief Netmap interface implementation
 * @details Provides C++ wrapper for netmap interface operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <netmap/interface.hpp>
#include <netmap/ring.hpp>

namespace libfreebsdnet::netmap {

  // NetmapConfig implementation
  NetmapConfig::NetmapConfig(const std::string &interface, uint32_t numRxRings,
                             uint32_t numTxRings)
      : interface(interface), numRxRings(numRxRings), numTxRings(numTxRings),
        bufferSize(65536), pollMode(true) {}

  // NetmapInterface implementation
  class NetmapInterface::Impl {
  public:
    Impl() : open_(false), capturing_(false), lastError_("") {}

    bool open(const NetmapConfig &config) {
      config_ = config;
      open_ = true;
      // Simplified implementation - real implementation would use netmap API
      return true;
    }

    void close() {
      if (capturing_) {
        capturing_ = false;
      }
      open_ = false;
    }

    bool isOpen() const { return open_; }

    std::string getInterfaceName() const { return config_.interface; }

    std::unique_ptr<NetmapRing> getRxRing(uint32_t index) const {
      if (!open_) {
        return nullptr;
      }
      // Simplified implementation
      (void)index; // Suppress unused parameter warning
      return std::make_unique<NetmapRing>();
    }

    std::unique_ptr<NetmapRing> getTxRing(uint32_t index) const {
      if (!open_) {
        return nullptr;
      }
      // Simplified implementation
      (void)index; // Suppress unused parameter warning
      return std::make_unique<NetmapRing>();
    }

    uint32_t getNumRxRings() const { return config_.numRxRings; }

    uint32_t getNumTxRings() const { return config_.numTxRings; }

    bool startCapture(PacketCallback callback) {
      if (!open_) {
        lastError_ = "Interface not open";
        return false;
      }

      if (capturing_) {
        lastError_ = "Capture already in progress";
        return false;
      }

      capturing_ = true;
      callback_ = callback;

      // Simplified implementation - real implementation would start netmap
      // polling
      return true;
    }

    void stopCapture() { capturing_ = false; }

    bool isCapturing() const { return capturing_; }

    bool sendPacket(const uint8_t *packet, uint32_t length,
                    uint32_t ringIndex) {
      if (!open_) {
        lastError_ = "Interface not open";
        return false;
      }

      // Simplified implementation - real implementation would use netmap TX
      // ring
      (void)packet; // Suppress unused parameter warning
      (void)length;
      (void)ringIndex;
      return true;
    }

    bool sendPacket(const std::vector<uint8_t> &packet, uint32_t ringIndex) {
      return sendPacket(packet.data(), packet.size(), ringIndex);
    }

    std::unordered_map<std::string, uint64_t> getStatistics() const {
      std::unordered_map<std::string, uint64_t> stats;
      // Simplified implementation
      return stats;
    }

    std::string getLastError() const { return lastError_; }

  private:
    NetmapConfig config_;
    bool open_;
    bool capturing_;
    std::string lastError_;
    PacketCallback callback_;
  };

  NetmapInterface::NetmapInterface() : pImpl(std::make_unique<Impl>()) {}

  NetmapInterface::NetmapInterface(const NetmapConfig &config)
      : pImpl(std::make_unique<Impl>()) {
    pImpl->open(config);
  }

  NetmapInterface::~NetmapInterface() { pImpl->close(); }

  bool NetmapInterface::open(const NetmapConfig &config) {
    return pImpl->open(config);
  }

  void NetmapInterface::close() { pImpl->close(); }

  bool NetmapInterface::isOpen() const { return pImpl->isOpen(); }

  std::string NetmapInterface::getInterfaceName() const {
    return pImpl->getInterfaceName();
  }

  std::unique_ptr<NetmapRing> NetmapInterface::getRxRing(uint32_t index) const {
    return pImpl->getRxRing(index);
  }

  std::unique_ptr<NetmapRing> NetmapInterface::getTxRing(uint32_t index) const {
    return pImpl->getTxRing(index);
  }

  uint32_t NetmapInterface::getNumRxRings() const {
    return pImpl->getNumRxRings();
  }

  uint32_t NetmapInterface::getNumTxRings() const {
    return pImpl->getNumTxRings();
  }

  bool NetmapInterface::startCapture(PacketCallback callback) {
    return pImpl->startCapture(callback);
  }

  void NetmapInterface::stopCapture() { pImpl->stopCapture(); }

  bool NetmapInterface::isCapturing() const { return pImpl->isCapturing(); }

  bool NetmapInterface::sendPacket(const uint8_t *packet, uint32_t length,
                                   uint32_t ringIndex) {
    return pImpl->sendPacket(packet, length, ringIndex);
  }

  bool NetmapInterface::sendPacket(const std::vector<uint8_t> &packet,
                                   uint32_t ringIndex) {
    return pImpl->sendPacket(packet, ringIndex);
  }

  std::unordered_map<std::string, uint64_t>
  NetmapInterface::getStatistics() const {
    return pImpl->getStatistics();
  }

  std::string NetmapInterface::getLastError() const {
    return pImpl->getLastError();
  }

} // namespace libfreebsdnet::netmap
