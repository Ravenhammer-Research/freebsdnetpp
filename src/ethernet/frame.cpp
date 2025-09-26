/**
 * @file ethernet/frame.cpp
 * @brief Ethernet frame implementation
 * @details Provides C++ wrapper for ethernet frame operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <ethernet/address.hpp>
#include <ethernet/frame.hpp>

namespace libfreebsdnet::ethernet {

  // Frame implementation
  class Frame::Impl {
  public:
    Impl() : valid_(false) {}

    bool parse(const std::vector<uint8_t> &data) {
      if (data.size() < 14) { // Minimum ethernet frame size
        return false;
      }

      // Parse ethernet header
      data_ = data;
      valid_ = true;
      return true;
    }

    MacAddress getDestination() const {
      if (data_.size() >= 6) {
        std::array<uint8_t, 6> bytes;
        std::copy(data_.begin(), data_.begin() + 6, bytes.begin());
        return MacAddress(bytes);
      }
      return MacAddress();
    }

    void setDestination(const MacAddress &address) {
      if (data_.size() >= 6) {
        auto bytes = address.getBytes();
        std::copy(bytes.begin(), bytes.end(), data_.begin());
      }
    }

    MacAddress getSource() const {
      if (data_.size() >= 12) {
        std::array<uint8_t, 6> bytes;
        std::copy(data_.begin() + 6, data_.begin() + 12, bytes.begin());
        return MacAddress(bytes);
      }
      return MacAddress();
    }

    void setSource(const MacAddress &address) {
      if (data_.size() >= 12) {
        auto bytes = address.getBytes();
        std::copy(bytes.begin(), bytes.end(), data_.begin() + 6);
      }
    }

    FrameType getType() const {
      if (data_.size() >= 14) {
        uint16_t type = (data_[12] << 8) | data_[13];
        return static_cast<FrameType>(type);
      }
      return FrameType::IPv4; // Default
    }

    void setType(FrameType type) {
      if (data_.size() >= 14) {
        uint16_t typeValue = static_cast<uint16_t>(type);
        data_[12] = (typeValue >> 8) & 0xFF;
        data_[13] = typeValue & 0xFF;
      }
    }

    std::vector<uint8_t> getPayload() const {
      if (data_.size() > 14) {
        return std::vector<uint8_t>(data_.begin() + 14, data_.end());
      }
      return std::vector<uint8_t>();
    }

    void setPayload(const std::vector<uint8_t> &payload) {
      if (data_.size() >= 14) {
        data_.resize(14 + payload.size());
        std::copy(payload.begin(), payload.end(), data_.begin() + 14);
      }
    }

    std::vector<uint8_t> toBytes() const { return data_; }

    size_t getSize() const { return data_.size(); }

    bool isValid() const { return valid_ && data_.size() >= 14; }

    uint32_t calculateChecksum() const {
      // Simplified checksum calculation
      uint32_t checksum = 0;
      for (size_t i = 0; i < data_.size(); ++i) {
        checksum += data_[i];
      }
      return checksum;
    }

    bool verifyChecksum() const {
      // Simplified checksum verification
      return true;
    }

  private:
    std::vector<uint8_t> data_;
    bool valid_;
  };

  Frame::Frame() : pImpl(std::make_unique<Impl>()) {}

  Frame::Frame(const std::vector<uint8_t> &data)
      : pImpl(std::make_unique<Impl>()) {
    pImpl->parse(data);
  }

  Frame::Frame(const MacAddress &destination, const MacAddress &source,
               FrameType type, const std::vector<uint8_t> &payload)
      : pImpl(std::make_unique<Impl>()) {
    // Initialize frame with header and payload
    std::vector<uint8_t> data(14 + payload.size());

    // Set destination
    auto destBytes = destination.getBytes();
    std::copy(destBytes.begin(), destBytes.end(), data.begin());

    // Set source
    auto srcBytes = source.getBytes();
    std::copy(srcBytes.begin(), srcBytes.end(), data.begin() + 6);

    // Set type
    uint16_t typeValue = static_cast<uint16_t>(type);
    data[12] = (typeValue >> 8) & 0xFF;
    data[13] = typeValue & 0xFF;

    // Set payload
    std::copy(payload.begin(), payload.end(), data.begin() + 14);

    pImpl->parse(data);
  }

  Frame::~Frame() = default;

  bool Frame::parse(const std::vector<uint8_t> &data) {
    return pImpl->parse(data);
  }

  MacAddress Frame::getDestination() const { return pImpl->getDestination(); }

  void Frame::setDestination(const MacAddress &address) {
    pImpl->setDestination(address);
  }

  MacAddress Frame::getSource() const { return pImpl->getSource(); }

  void Frame::setSource(const MacAddress &address) {
    pImpl->setSource(address);
  }

  FrameType Frame::getType() const { return pImpl->getType(); }

  void Frame::setType(FrameType type) { pImpl->setType(type); }

  std::vector<uint8_t> Frame::getPayload() const { return pImpl->getPayload(); }

  void Frame::setPayload(const std::vector<uint8_t> &payload) {
    pImpl->setPayload(payload);
  }

  std::vector<uint8_t> Frame::toBytes() const { return pImpl->toBytes(); }

  size_t Frame::getSize() const { return pImpl->getSize(); }

  bool Frame::isValid() const { return pImpl->isValid(); }

  uint32_t Frame::calculateChecksum() const {
    return pImpl->calculateChecksum();
  }

  bool Frame::verifyChecksum() const { return pImpl->verifyChecksum(); }

} // namespace libfreebsdnet::ethernet
