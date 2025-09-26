/**
 * @file interface/factory.cpp
 * @brief Interface factory implementation
 * @details Implementation of interface factory functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <cstring>
#include <interface/bridge.hpp>
#include <interface/carp.hpp>
#include <interface/ethernet.hpp>
#include <interface/factory.hpp>
#include <interface/l2vlan.hpp>
#include <interface/lagg.hpp>
#include <interface/pflog.hpp>
#include <interface/pfsync.hpp>
#include <interface/tunnel.hpp>
#include <interface/vlan.hpp>
#include <interface/wireless.hpp>
#include <memory>
#include <net/if.h>
#include <net/if_media.h>
#include <net80211/ieee80211_ioctl.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace libfreebsdnet::interface {

  std::unique_ptr<Interface>
  InterfaceFactory::createInterface(const std::string &name, unsigned int index,
                                    int flags) {
    InterfaceType type = getInterfaceType(name, flags);
    return createInterface(name, index, flags, type);
  }

  std::unique_ptr<Interface>
  InterfaceFactory::createInterface(const std::string &name, unsigned int index,
                                    int flags, InterfaceType type) {
    switch (type) {
    case InterfaceType::ETHERNET:
      return std::make_unique<EthernetInterface>(name, index, flags);
    case InterfaceType::LOOPBACK:
      return std::make_unique<EthernetInterface>(
          name, index, flags); // Use EthernetInterface for loopback
    case InterfaceType::BRIDGE:
      return std::make_unique<BridgeInterface>(name, index, flags);
    case InterfaceType::VLAN:
      return std::make_unique<VlanInterface>(name, index, flags);
    case InterfaceType::TUNNEL:
    case InterfaceType::TAP:
    case InterfaceType::TUN:
    case InterfaceType::STF:
    case InterfaceType::ENCAP:
      return std::make_unique<TunnelInterface>(name, index, flags);
    case InterfaceType::LAGG:
      return std::make_unique<LagInterface>(name, index, flags);
    case InterfaceType::PFSYNC:
      return std::make_unique<PfsyncInterface>(name, index, flags);
    case InterfaceType::PFLOG:
      return std::make_unique<PflogInterface>(name, index, flags);
    case InterfaceType::CARP:
      return std::make_unique<CarpInterface>(name, index, flags);
    case InterfaceType::L2VLAN:
      return std::make_unique<L2VlanInterface>(name, index, flags);
    case InterfaceType::EPAIR:
      return std::make_unique<EthernetInterface>(name, index, flags); // Use EthernetInterface as base
    case InterfaceType::INFINIBAND_LAG:
      return std::make_unique<LagInterface>(name, index, flags);
    case InterfaceType::IEEE8023AD_LAG:
      return std::make_unique<LagInterface>(name, index, flags);
    case InterfaceType::WIRELESS:
      return std::make_unique<WirelessInterface>(name, index, flags);
    default:
      return nullptr;
    }
  }

  InterfaceType InterfaceFactory::getInterfaceType(const std::string &name,
                                                   int flags) {
    InterfaceType nameType = getTypeFromName(name);
    if (nameType != InterfaceType::UNKNOWN) {
      return nameType;
    }
    return getTypeFromFlags(flags);
  }

  InterfaceType InterfaceFactory::getInterfaceType(const std::string &name) {
    return getTypeFromName(name);
  }

  bool InterfaceFactory::isSupported(InterfaceType type) {
    switch (type) {
    case InterfaceType::ETHERNET:
    case InterfaceType::LOOPBACK:
    case InterfaceType::BRIDGE:
    case InterfaceType::VLAN:
    case InterfaceType::TUNNEL:
    case InterfaceType::TAP:
    case InterfaceType::TUN:
    case InterfaceType::STF:
    case InterfaceType::ENCAP:
    case InterfaceType::LAGG:
    case InterfaceType::PFSYNC:
    case InterfaceType::PFLOG:
    case InterfaceType::CARP:
      return true;
    case InterfaceType::L2VLAN:
      return true;
    case InterfaceType::EPAIR:
      return true;
    case InterfaceType::INFINIBAND_LAG:
      return true;
    case InterfaceType::IEEE8023AD_LAG:
      return true;
    case InterfaceType::WIRELESS:
      return true;
    default:
      return false;
    }
  }

  std::vector<InterfaceType> InterfaceFactory::getSupportedTypes() {
    return {InterfaceType::ETHERNET,
            InterfaceType::LOOPBACK,
            InterfaceType::BRIDGE,
            InterfaceType::VLAN,
            InterfaceType::TUNNEL,
            InterfaceType::TAP,
            InterfaceType::TUN,
            InterfaceType::STF,
            InterfaceType::ENCAP,
            InterfaceType::LAGG,
            InterfaceType::PFSYNC,
            InterfaceType::PFLOG,
            InterfaceType::CARP,
            InterfaceType::L2VLAN,
            InterfaceType::EPAIR,
            InterfaceType::INFINIBAND_LAG,
            InterfaceType::IEEE8023AD_LAG,
            InterfaceType::WIRELESS};
  }

  InterfaceType InterfaceFactory::getTypeFromName(const std::string &name) {
    if (name.empty()) {
      return InterfaceType::UNKNOWN;
    }

    // Check interface name prefixes
    if (name.substr(0, 2) == "lo") {
      return InterfaceType::LOOPBACK;
    }

    if (name.substr(0, 5) == "epair") {
      return InterfaceType::EPAIR;
    }

    if (name.substr(0, 3) == "eth" || name.substr(0, 2) == "em" ||
        name.substr(0, 3) == "igb" || name.substr(0, 3) == "ixg" ||
        name.substr(0, 3) == "bge" || name.substr(0, 3) == "fxp") {
      return InterfaceType::ETHERNET;
    }

    if (name.substr(0, 6) == "bridge") {
      return InterfaceType::BRIDGE;
    }

    if (name.substr(0, 4) == "vlan") {
      return InterfaceType::VLAN;
    }

    if (name.substr(0, 3) == "gif") {
      return InterfaceType::TUNNEL;
    }

    if (name.substr(0, 3) == "tap") {
      return InterfaceType::TAP;
    }

    if (name.substr(0, 3) == "tun") {
      return InterfaceType::TUN;
    }

    if (name.substr(0, 3) == "stf") {
      return InterfaceType::STF;
    }

    if (name.substr(0, 4) == "lagg") {
      return InterfaceType::LAGG;
    }

    if (name.substr(0, 5) == "pfsync") {
      return InterfaceType::PFSYNC;
    }

    if (name.substr(0, 5) == "pflog") {
      return InterfaceType::PFLOG;
    }

    if (name.substr(0, 4) == "carp") {
      return InterfaceType::CARP;
    }

    if (name.substr(0, 6) == "l2vlan") {
      return InterfaceType::L2VLAN;
    }

    if (name.substr(0, 2) == "ib" && name.find("lag") != std::string::npos) {
      return InterfaceType::INFINIBAND_LAG;
    }

    if (name.substr(0, 4) == "lagg" && name.find("lacp") != std::string::npos) {
      return InterfaceType::IEEE8023AD_LAG;
    }

    if (name.substr(0, 4) == "wlan") {
      return InterfaceType::WIRELESS;
    }

    if (name.substr(0, 2) == "lo") {
      return InterfaceType::LOOPBACK;
    }

    return InterfaceType::UNKNOWN;
  }

  InterfaceType InterfaceFactory::getTypeFromFlags(int flags) {
    // Real interface type detection based on flags and system queries

    // Check for loopback interface
    if (flags & IFF_LOOPBACK) {
      return InterfaceType::LOOPBACK;
    }

    // Check for point-to-point interface
    if (flags & IFF_POINTOPOINT) {
      return InterfaceType::PPP;
    }

    // Check for broadcast interface (most Ethernet interfaces)
    if (flags & IFF_BROADCAST) {
      return InterfaceType::ETHERNET;
    }

    // Check for wireless interface using real FreeBSD wireless system calls
    if (flags & IFF_UP && !(flags & IFF_LOOPBACK)) {
      int sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock >= 0) {
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));

        // Check for IEEE 802.11 wireless interface using proper FreeBSD ioctls
        if (ioctl(sock, SIOCGIFMEDIA, &ifr) == 0) {
          // Check if media type indicates IEEE 802.11 wireless
          if (ifr.ifr_media & IFM_IEEE80211) {
            close(sock);
            return InterfaceType::WIRELESS;
          }
        }

        // Try IEEE 802.11 specific ioctls
        struct ieee80211req ireq;
        std::memset(&ireq, 0, sizeof(ireq));
        std::strncpy(ireq.i_name, "wlan0",
                     IFNAMSIZ - 1); // Try common wireless interface

        if (ioctl(sock, SIOCG80211, &ireq) == 0) {
          close(sock);
          return InterfaceType::WIRELESS;
        }

        close(sock);
      }

      // Default to Ethernet for broadcast interfaces
      return InterfaceType::ETHERNET;
    }

    return InterfaceType::UNKNOWN;
  }

} // namespace libfreebsdnet::interface
