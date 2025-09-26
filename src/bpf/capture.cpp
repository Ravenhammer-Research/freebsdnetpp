/**
 * @file bpf/capture.cpp
 * @brief BPF packet capture implementation
 * @details Provides C++ wrapper for BPF packet capture operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <bpf/capture.hpp>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <net/bpf.h>
#include <net/if.h>
#include <poll.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

namespace libfreebsdnet::bpf {

  // CapturedPacket implementation
  CapturedPacket::CapturedPacket(
      const std::vector<uint8_t> &data,
      const std::chrono::system_clock::time_point &timestamp, uint32_t length,
      uint32_t caplen, const std::string &interface)
      : data(data), timestamp(timestamp), length(length), caplen(caplen),
        interface(interface) {}

  // PacketCapture implementation
  class PacketCapture::Impl {
  public:
    Impl() : fd_(-1), capturing_(false), lastError_("") {}

    ~Impl() {
      if (fd_ >= 0) {
        ::close(fd_);
      }
    }

    bool open(const std::string &interfaceName, size_t bufferSize) {
      if (fd_ >= 0) {
        ::close(fd_);
      }

      // Open BPF device
      for (int i = 0; i < 10; ++i) {
        std::string device = "/dev/bpf" + std::to_string(i);
        fd_ = ::open(device.c_str(), O_RDWR);
        if (fd_ >= 0) {
          break;
        }
      }

      if (fd_ < 0) {
        lastError_ = "Failed to open BPF device";
        return false;
      }

      // Bind to interface
      struct ifreq ifr;
      std::strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
      ifr.ifr_name[IFNAMSIZ - 1] = '\0';

      if (ioctl(fd_, BIOCSETIF, &ifr) < 0) {
        lastError_ = "Failed to bind to interface: " + interfaceName;
        ::close(fd_);
        fd_ = -1;
        return false;
      }

      // Set buffer size
      uint32_t bufsize = bufferSize;
      if (ioctl(fd_, BIOCSBLEN, &bufsize) < 0) {
        lastError_ = "Failed to set buffer size";
        ::close(fd_);
        fd_ = -1;
        return false;
      }

      interfaceName_ = interfaceName;
      return true;
    }

    void close() {
      if (fd_ >= 0) {
        capturing_ = false;
        ::close(fd_);
        fd_ = -1;
      }
    }

    bool setFilter(const Filter &filter) {
      if (fd_ < 0) {
        lastError_ = "BPF device not open";
        return false;
      }

      if (!filter.isValid()) {
        lastError_ = "Invalid filter";
        return false;
      }

      auto instructions = filter.getInstructions();
      struct bpf_program prog;
      prog.bf_len = instructions.size();
      prog.bf_insns = reinterpret_cast<struct bpf_insn *>(instructions.data());

      if (ioctl(fd_, BIOCSETF, &prog) < 0) {
        lastError_ = "Failed to set BPF filter";
        return false;
      }

      return true;
    }

    bool setFilter(const std::string &expression) {
      Filter filter;
      if (!filter.compile(expression)) {
        lastError_ = filter.getLastError();
        return false;
      }

      return setFilter(filter);
    }

    bool startCapture(PacketCallback callback) {
      if (fd_ < 0) {
        lastError_ = "BPF device not open";
        return false;
      }

      if (capturing_) {
        lastError_ = "Capture already in progress";
        return false;
      }

      capturing_ = true;

      // Start capture thread
      captureThread_ = std::thread([this, callback]() {
        while (capturing_) {
          struct bpf_hdr *bpf_hdr;
          char buffer[65536];

          ssize_t n = read(fd_, buffer, sizeof(buffer));
          if (n < 0) {
            if (errno == EINTR || errno == EAGAIN) {
              continue;
            }
            break;
          }

          if (n == 0) {
            continue;
          }

          // Parse BPF packets
          char *ptr = buffer;
          while (ptr < buffer + n) {
            bpf_hdr = reinterpret_cast<struct bpf_hdr *>(ptr);

            // Extract packet data
            std::vector<uint8_t> packetData(
                reinterpret_cast<uint8_t *>(ptr + bpf_hdr->bh_hdrlen),
                reinterpret_cast<uint8_t *>(ptr + bpf_hdr->bh_hdrlen) +
                    bpf_hdr->bh_caplen);

            auto timestamp = std::chrono::system_clock::now();
            CapturedPacket packet(packetData, timestamp, bpf_hdr->bh_datalen,
                                  bpf_hdr->bh_caplen, interfaceName_);

            if (!callback(packet)) {
              capturing_ = false;
              break;
            }

            ptr += BPF_WORDALIGN(bpf_hdr->bh_hdrlen + bpf_hdr->bh_caplen);
          }
        }
      });

      return true;
    }

    bool startCapture(PacketCallback callback,
                      const std::chrono::milliseconds &timeout) {
      if (fd_ < 0) {
        lastError_ = "BPF device not open";
        return false;
      }

      if (capturing_) {
        lastError_ = "Capture already in progress";
        return false;
      }

      // Store callback for later use
      capturing_ = true;

      // Start capture thread with timeout support
      captureThread_ = std::thread([this, timeout, callback]() {
        std::vector<uint8_t> buffer(65536); // 64KB buffer

        while (capturing_) {
          struct pollfd pfd;
          pfd.fd = fd_;
          pfd.events = POLLIN;

          int timeout_ms = timeout.count();
          int ret = poll(&pfd, 1, timeout_ms);

          if (ret < 0) {
            if (errno == EINTR) {
              continue; // Interrupted by signal, retry
            }
            break; // Error occurred
          }

          if (ret == 0) {
            // Timeout occurred
            continue;
          }

          if (pfd.revents & POLLIN) {
            ssize_t n = read(fd_, buffer.data(), buffer.size());
            if (n > 0) {
              // Parse BPF header and create packet
              struct bpf_hdr *bh =
                  reinterpret_cast<struct bpf_hdr *>(buffer.data());
              if (n >= static_cast<ssize_t>(bh->bh_hdrlen + bh->bh_caplen)) {
                std::vector<uint8_t> packet_data(buffer.data() + bh->bh_hdrlen,
                                                 buffer.data() + bh->bh_hdrlen +
                                                     bh->bh_caplen);

                auto timestamp = std::chrono::system_clock::now();
                CapturedPacket packet(packet_data, timestamp, bh->bh_datalen,
                                      bh->bh_caplen, interfaceName_);

                if (callback) {
                  callback(packet);
                }
              }
            }
          }
        }
      });

      return true;
    }

    void stopCapture() {
      capturing_ = false;
      if (captureThread_.joinable()) {
        captureThread_.join();
      }
    }

    bool isCapturing() const { return capturing_; }

    std::unordered_map<std::string, uint64_t> getStatistics() const {
      std::unordered_map<std::string, uint64_t> stats;

      if (fd_ < 0) {
        return stats;
      }

      struct bpf_stat bstat;
      if (ioctl(fd_, BIOCGSTATS, &bstat) >= 0) {
        stats["packets_received"] = bstat.bs_recv;
        stats["packets_dropped"] = bstat.bs_drop;
      }

      return stats;
    }

    bool setPromiscuousMode(bool enabled) {
      if (fd_ < 0) {
        lastError_ = "BPF device not open";
        return false;
      }

      uint32_t mode = enabled ? 1 : 0;
      if (ioctl(fd_, BIOCPROMISC, &mode) < 0) {
        lastError_ = "Failed to set promiscuous mode";
        return false;
      }

      return true;
    }

    std::string getLastError() const { return lastError_; }

  private:
    int fd_;
    bool capturing_;
    std::string lastError_;
    std::string interfaceName_;
    std::thread captureThread_;
  };

  PacketCapture::PacketCapture() : pImpl(std::make_unique<Impl>()) {}

  PacketCapture::~PacketCapture() { pImpl->stopCapture(); }

  bool PacketCapture::open(const std::string &interfaceName,
                           size_t bufferSize) {
    return pImpl->open(interfaceName, bufferSize);
  }

  void PacketCapture::close() { pImpl->close(); }

  bool PacketCapture::setFilter(const Filter &filter) {
    return pImpl->setFilter(filter);
  }

  bool PacketCapture::setFilter(const std::string &expression) {
    return pImpl->setFilter(expression);
  }

  bool PacketCapture::startCapture(PacketCallback callback) {
    return pImpl->startCapture(callback);
  }

  bool PacketCapture::startCapture(PacketCallback callback,
                                   const std::chrono::milliseconds &timeout) {
    return pImpl->startCapture(callback, timeout);
  }

  void PacketCapture::stopCapture() { pImpl->stopCapture(); }

  bool PacketCapture::isCapturing() const { return pImpl->isCapturing(); }

  std::unordered_map<std::string, uint64_t>
  PacketCapture::getStatistics() const {
    return pImpl->getStatistics();
  }

  bool PacketCapture::setPromiscuousMode(bool enabled) {
    return pImpl->setPromiscuousMode(enabled);
  }

  std::string PacketCapture::getLastError() const {
    return pImpl->getLastError();
  }

} // namespace libfreebsdnet::bpf
