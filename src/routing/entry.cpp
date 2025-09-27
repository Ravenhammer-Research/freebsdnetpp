/**
 * @file routing/entry.cpp
 * @brief Routing entry implementation
 * @details Provides C++ wrapper for FreeBSD routing entries
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <routing/entry.hpp>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace libfreebsdnet::routing {

  // RoutingEntryInfo implementation
  RoutingEntryInfo::RoutingEntryInfo(const std::string &dest,
                                     const std::string &gw,
                                     const std::string &iface, uint16_t flags,
                                     uint32_t metric)
      : destination(dest), gateway(gw), interface(iface), netmask(""),
        flags(flags), metric(metric), mtu(0),
        lastUpdated(std::chrono::system_clock::now()) {}

  // RoutingEntry implementation
  class RoutingEntry::Impl {
  public:
    Impl() : info_() {}

    explicit Impl(const RoutingEntryInfo &info) : info_(info) {}

    std::string getDestination() const { return info_.destination; }

    std::string getGateway() const { return info_.gateway; }

    std::string getInterface() const { return info_.interface; }

    uint16_t getFlags() const { return info_.flags; }

    std::vector<RouteFlag> getFlagList() const {
      std::vector<RouteFlag> flags;
      uint32_t raw_flags = info_.flags; // Use uint32_t to handle all flags
      
      if (raw_flags & RTF_UP) flags.push_back(RouteFlag::UP);
      if (raw_flags & RTF_GATEWAY) flags.push_back(RouteFlag::GATEWAY);
      if (raw_flags & RTF_HOST) flags.push_back(RouteFlag::HOST);
      if (raw_flags & RTF_REJECT) flags.push_back(RouteFlag::REJECT);
      if (raw_flags & RTF_DYNAMIC) flags.push_back(RouteFlag::DYNAMIC);
      if (raw_flags & RTF_MODIFIED) flags.push_back(RouteFlag::MODIFIED);
      if (raw_flags & RTF_DONE) flags.push_back(RouteFlag::DONE);
      if (raw_flags & RTF_XRESOLVE) flags.push_back(RouteFlag::XRESOLVE);
      if (raw_flags & RTF_LLINFO) flags.push_back(RouteFlag::LLINFO);
      if (raw_flags & RTF_STATIC) flags.push_back(RouteFlag::STATIC);
      if (raw_flags & RTF_BLACKHOLE) flags.push_back(RouteFlag::BLACKHOLE);
      if (raw_flags & RTF_PROTO2) flags.push_back(RouteFlag::PROTO2);
      if (raw_flags & RTF_PROTO1) flags.push_back(RouteFlag::PROTO1);
      if (raw_flags & RTF_PROTO3) flags.push_back(RouteFlag::PROTO3);
      if (raw_flags & RTF_FIXEDMTU) flags.push_back(RouteFlag::FIXEDMTU);
      if (raw_flags & RTF_PINNED) flags.push_back(RouteFlag::PINNED);
      
      return flags;
    }

    uint32_t getMetric() const { return info_.metric; }

    uint32_t getMtu() const { return info_.mtu; }

    std::string getNetmask() const { return info_.netmask; }

    bool isActive() const { return (info_.flags & RTF_UP) != 0; }

    bool isDefault() const {
      return info_.destination == "0.0.0.0/0" || info_.destination == "::/0";
    }

    bool isHost() const {
      // Check if destination is a host route (no CIDR notation)
      return info_.destination.find('/') == std::string::npos;
    }

    bool isNetwork() const { return !isHost(); }

    RoutingEntryInfo getInfo() const { return info_; }

    void updateInfo(const RoutingEntryInfo &info) { info_ = info; }

  private:
    RoutingEntryInfo info_;
  };

  RoutingEntry::RoutingEntry() : pImpl(std::make_unique<Impl>()) {}

  RoutingEntry::RoutingEntry(const RoutingEntryInfo &info)
      : pImpl(std::make_unique<Impl>(info)) {}

  RoutingEntry::~RoutingEntry() = default;

  std::string RoutingEntry::getDestination() const {
    return pImpl->getDestination();
  }

  std::string RoutingEntry::getGateway() const { return pImpl->getGateway(); }

  std::string RoutingEntry::getInterface() const {
    return pImpl->getInterface();
  }

  uint16_t RoutingEntry::getFlags() const { return pImpl->getFlags(); }

  std::vector<RouteFlag> RoutingEntry::getFlagList() const {
    return pImpl->getFlagList();
  }

  uint32_t RoutingEntry::getMetric() const { return pImpl->getMetric(); }

  uint32_t RoutingEntry::getMtu() const { return pImpl->getMtu(); }

  std::string RoutingEntry::getNetmask() const { return pImpl->getNetmask(); }

  bool RoutingEntry::isActive() const { return pImpl->isActive(); }

  bool RoutingEntry::isDefault() const { return pImpl->isDefault(); }

  bool RoutingEntry::isHost() const { return pImpl->isHost(); }

  bool RoutingEntry::isNetwork() const { return pImpl->isNetwork(); }

  RoutingEntryInfo RoutingEntry::getInfo() const { return pImpl->getInfo(); }

  void RoutingEntry::updateInfo(const RoutingEntryInfo &info) {
    pImpl->updateInfo(info);
  }

} // namespace libfreebsdnet::routing
