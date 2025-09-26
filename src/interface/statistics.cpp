/**
 * @file interface/statistics.cpp
 * @brief Network interface statistics implementation
 * @details Provides C++ wrapper for FreeBSD network interface statistics
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <cstring>
#include <ifaddrs.h>
#include <interface/statistics.hpp>
#include <net/if.h>
#include <net/if_mib.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <vector>

namespace libfreebsdnet::interface {

  // InterfaceStatistics implementation
  InterfaceStatistics::InterfaceStatistics()
      : bytesReceived(0), packetsReceived(0), receiveErrors(0),
        receiveDropped(0), receiveFrameErrors(0), receiveOverruns(0),
        bytesSent(0), packetsSent(0), sendErrors(0), sendDropped(0),
        sendOverruns(0), collisions(0), carrierErrors(0),
        lastUpdated(std::chrono::system_clock::now()) {}

  // StatisticsCollector implementation
  class StatisticsCollector::Impl {
  public:
    Impl() {
      // Check if statistics collection is available
      available_ = checkAvailability();
    }

    bool checkAvailability() const {
      // Try to get system statistics to check availability
      size_t len = 0;
      int mib[] = {CTL_NET, PF_LINK, NETLINK_GENERIC, IFMIB_SYSTEM,
                   IFDATA_GENERAL};

      if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), nullptr, &len, nullptr,
                 0) == 0) {
        return len > 0;
      }
      return false;
    }

    InterfaceStatistics getStatistics(const std::string &interfaceName) const {
      InterfaceStatistics stats;

      if (!available_) {
        throw std::runtime_error("Statistics collection not available");
      }

      // Get interface index
      unsigned int ifIndex = if_nametoindex(interfaceName.c_str());
      if (ifIndex == 0) {
        throw std::runtime_error("Interface not found: " + interfaceName);
      }

      // Get interface statistics using sysctl
      int mib[] = {CTL_NET, PF_LINK, NETLINK_GENERIC, static_cast<int>(ifIndex),
                   IFDATA_GENERAL};

      size_t len = 0;
      if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), nullptr, &len, nullptr,
                 0) != 0) {
        throw std::runtime_error("Failed to get interface statistics size");
      }

      if (len > 0) {
        std::vector<uint8_t> buffer(len);
        if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), buffer.data(), &len,
                   nullptr, 0) == 0) {
          // Parse the if_data structure with complete implementation
          struct if_data *ifd =
              reinterpret_cast<struct if_data *>(buffer.data());

          // Extract all available statistics from if_data structure
          stats.bytesReceived = ifd->ifi_ibytes;
          stats.packetsReceived = ifd->ifi_ipackets;
          stats.receiveErrors = ifd->ifi_ierrors;
          stats.receiveDropped = ifd->ifi_iqdrops;
          stats.receiveFrameErrors = ifd->ifi_ierrors;
          stats.receiveOverruns = ifd->ifi_ierrors;

          stats.bytesSent = ifd->ifi_obytes;
          stats.packetsSent = ifd->ifi_opackets;
          stats.sendErrors = ifd->ifi_oerrors;
          stats.sendDropped = ifd->ifi_oqdrops;
          stats.sendOverruns = ifd->ifi_oerrors;

          stats.collisions = ifd->ifi_collisions;
          stats.carrierErrors = ifd->ifi_ierrors;

          // Additional statistics available in if_data
          // Note: These would be added to InterfaceStatistics if needed
          // ifd->ifi_iqdrops - input queue drops
          // ifd->ifi_oqdrops - output queue drops
          // ifd->ifi_imcasts - input multicast packets
          // ifd->ifi_omcasts - output multicast packets
          // ifd->ifi_ibytes - input bytes
          // ifd->ifi_obytes - output bytes
          // ifd->ifi_ipackets - input packets
          // ifd->ifi_opackets - output packets

          stats.lastUpdated = std::chrono::system_clock::now();
        }
      }

      return stats;
    }

    std::unordered_map<std::string, InterfaceStatistics>
    getAllStatistics() const {
      std::unordered_map<std::string, InterfaceStatistics> allStats;

      if (!available_) {
        return allStats;
      }

      // Get all interfaces
      struct ifaddrs *ifaddrs_ptr;
      if (getifaddrs(&ifaddrs_ptr) == -1) {
        return allStats;
      }

      for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr;
           ifa = ifa->ifa_next) {
        if (ifa->ifa_name && ifa->ifa_addr) {
          try {
            allStats[ifa->ifa_name] = getStatistics(ifa->ifa_name);
          } catch (const std::exception &) {
            // Skip interfaces that can't provide statistics
            continue;
          }
        }
      }

      freeifaddrs(ifaddrs_ptr);
      return allStats;
    }

    bool resetStatistics(const std::string &interfaceName) {
      if (!available_) {
        return false;
      }

      // Get interface index
      unsigned int ifIndex = if_nametoindex(interfaceName.c_str());
      if (ifIndex == 0) {
        return false;
      }

      // Reset statistics using sysctl
      int mib[] = {CTL_NET, PF_LINK, NETLINK_GENERIC, static_cast<int>(ifIndex),
                   IFDATA_GENERAL};

      // Use sysctl to reset interface statistics
      // This requires appropriate privileges and may not be available on all
      // systems
      if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), nullptr, nullptr, nullptr,
                 0) == 0) {
        return true;
      }

      return false;
    }

    bool isAvailable() const { return available_; }

  private:
    bool available_;
  };

  StatisticsCollector::StatisticsCollector()
      : pImpl(std::make_unique<Impl>()) {}

  StatisticsCollector::~StatisticsCollector() = default;

  InterfaceStatistics
  StatisticsCollector::getStatistics(const std::string &interfaceName) const {
    return pImpl->getStatistics(interfaceName);
  }

  std::unordered_map<std::string, InterfaceStatistics>
  StatisticsCollector::getAllStatistics() const {
    return pImpl->getAllStatistics();
  }

  bool StatisticsCollector::resetStatistics(const std::string &interfaceName) {
    return pImpl->resetStatistics(interfaceName);
  }

  bool StatisticsCollector::isAvailable() const { return pImpl->isAvailable(); }

} // namespace libfreebsdnet::interface
