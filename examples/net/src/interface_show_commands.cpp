/**
 * @file interface_show_commands.cpp
 * @brief Net tool interface show command implementations
 * @details Implementation of interface show-related command handlers for the
 * net tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/bridge.hpp>
#include <interface/gif.hpp>
#include <interface/lagg.hpp>
#include <interface/vnet.hpp>
#include <interface/wireless.hpp>
#include <iostream>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net_tool.hpp>
#include <sstream>
#include <system/config.hpp>

namespace net {

  // Helper function to format interface flags as single letters
  std::string
  formatFlags(const std::vector<libfreebsdnet::interface::Flag> &flags) {
    std::string result = "";
    for (const auto &flag : flags) {
      switch (flag) {
      case libfreebsdnet::interface::Flag::UP:
        result += "U";
        break;
      case libfreebsdnet::interface::Flag::RUNNING:
        result += "R";
        break;
      case libfreebsdnet::interface::Flag::BROADCAST:
        result += "B";
        break;
      case libfreebsdnet::interface::Flag::MULTICAST:
        result += "M";
        break;
      case libfreebsdnet::interface::Flag::LOOPBACK:
        result += "L";
        break;
      case libfreebsdnet::interface::Flag::POINTOPOINT:
        result += "P";
        break;
      case libfreebsdnet::interface::Flag::SIMPLEX:
        result += "S";
        break;
      case libfreebsdnet::interface::Flag::DRV_RUNNING:
        result += "D";
        break;
      case libfreebsdnet::interface::Flag::NOARP:
        result += "A";
        break;
      case libfreebsdnet::interface::Flag::PROMISC:
        result += "p";
        break;
      case libfreebsdnet::interface::Flag::ALLMULTI:
        result += "a";
        break;
      case libfreebsdnet::interface::Flag::OACTIVE:
        result += "o";
        break;
      case libfreebsdnet::interface::Flag::LINK0:
        result += "0";
        break;
      case libfreebsdnet::interface::Flag::LINK1:
        result += "1";
        break;
      case libfreebsdnet::interface::Flag::LINK2:
        result += "2";
        break;
      default:
        break;
      }
    }
    return result.empty() ? "-" : result;
  }

  bool NetTool::handleShowInterfaces(const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning
    try {
      auto interfaces = interfaceManager.getInterfaces();

      if (interfaces.empty()) {
        printInfo("No interfaces found.");
        return true;
      }

      // Print flags legend at the top
      printInfo("Flags Legend:");
      printInfo("  U = UP, R = RUNNING, B = BROADCAST, M = MULTICAST");
      printInfo(
          "  L = LOOPBACK, P = POINTOPOINT, S = SIMPLEX, D = DRV_RUNNING");
      printInfo("  A = NOARP, p = PROMISC, a = ALLMULTI, o = OACTIVE");
      printInfo("  0/1/2 = LINK0/LINK1/LINK2");
      printInfo("");

      std::vector<std::vector<std::string>> data;
      std::vector<std::string> headers = {"Name",   "Type", "MTU",  "Address",
                                          "Status", "FIB",  "Flags"};

      for (const auto &interface : interfaces) {
        // Get interface type from the interface object
        libfreebsdnet::interface::InterfaceType interfaceType =
            interface->getType();
        std::string type_str;
        switch (interfaceType) {
        case libfreebsdnet::interface::InterfaceType::ETHERNET:
          type_str = "Ethernet";
          break;
        case libfreebsdnet::interface::InterfaceType::LOOPBACK:
          type_str = "Loopback";
          break;
        case libfreebsdnet::interface::InterfaceType::BRIDGE:
          type_str = "Bridge";
          break;
        case libfreebsdnet::interface::InterfaceType::WIRELESS:
          type_str = "IEEE80211";
          break;
        case libfreebsdnet::interface::InterfaceType::L2VLAN:
          type_str = "L2VLAN";
          break;
        case libfreebsdnet::interface::InterfaceType::EPAIR:
          type_str = "EthernetPair";
          break;
        case libfreebsdnet::interface::InterfaceType::LAGG:
          type_str = "LinkAggregate";
          break;
        case libfreebsdnet::interface::InterfaceType::GIF:
          type_str = "GenericTunnel";
          break;
        default:
          type_str = "Unknown";
          break;
        }

        std::string status = interface->isUp() ? "UP" : "DOWN";
        std::string flags_str = formatFlags(interface->getFlags());

        // Get addresses from the interface object
        auto addresses = interface->getAddresses();
        if (addresses.empty()) {
          // No addresses - show one row with "None"
          std::vector<std::string> row;
          row.push_back(interface->getName());
          row.push_back(type_str);
          row.push_back(std::to_string(interface->getMtu()));
          row.push_back("None");
          row.push_back(status);
          // Get FIB for this interface
          try {
            row.push_back(std::to_string(interface->getFib()));
          } catch (...) {
            row.push_back("0"); // Default FIB on error
          }
          row.push_back(flags_str);
          data.push_back(row);
        } else {
          // Get FIB for this interface (only need to do this once)
          std::string fib_str = "0"; // Default FIB
          try {
            fib_str = std::to_string(interface->getFib());
          } catch (...) {
            // Keep default FIB on error
          }

          // Show each address on a separate row
          for (size_t i = 0; i < addresses.size(); i++) {
            std::vector<std::string> row;
            row.push_back(i == 0 ? interface->getName()
                                 : ""); // Only show name on first row
            row.push_back(i == 0 ? type_str
                                 : ""); // Only show type on first row
            row.push_back(i == 0 ? std::to_string(interface->getMtu())
                                 : ""); // Only show MTU on first row
            row.push_back(addresses[i].getCidr());
            row.push_back(i == 0 ? status
                                 : ""); // Only show status on first row
            row.push_back(i == 0 ? fib_str : ""); // Only show FIB on first row
            row.push_back(i == 0 ? flags_str
                                 : ""); // Only show flags on first row
            data.push_back(row);
          }
        }
      }

      printTable(data, headers);
      return true;
    } catch (const std::exception &e) {
      printError("Failed to get interfaces: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleShowInterfaceInfo(const std::vector<std::string> &args) {
    if (args.size() < 3) {
      printError("Usage: show interface <name> [property]");
      return false;
    }

    std::string name = args[2];

    try {
      // Get interface info first
      auto info = interfaceManager.getInterface(name);
      if (!info) {
        printError("Interface not found: " + name);
        return false;
      }

      // Get interface object
      auto iface = interfaceManager.getInterface(name);

      if (!iface) {
        printError("Failed to get interface object for: " + name);
        return false;
      }

      if (args.size() > 3) {
        // Show specific property
        std::string property = args[3];
        if (property == "fib") {
          int fib = iface->getFib();
          printInfo("FIB: " + std::to_string(fib));
        } else if (property == "mtu") {
          int mtu = iface->getMtu();
          printInfo("MTU: " + std::to_string(mtu));
        } else if (property == "media") {
          int media = iface->getMedia();
          printInfo("Media: 0x" + std::to_string(media));
        } else if (property == "capabilities") {
          uint32_t caps = iface->getCapabilities();
          printInfo("Capabilities: 0x" + std::to_string(caps));
        } else if (property == "groups") {
          auto groups = iface->getGroups();
          if (groups.empty()) {
            printInfo("Groups: None");
          } else {
            std::string groupList;
            for (size_t i = 0; i < groups.size(); i++) {
              if (i > 0)
                groupList += ", ";
              groupList += groups[i];
            }
            printInfo("Groups: " + groupList);
          }
        } else if (property == "vnet") {
          auto vnetIface =
              dynamic_cast<libfreebsdnet::interface::VnetInterface *>(
                  iface.get());
          if (!vnetIface) {
            printError("Interface " + name +
                       " does not support VNET operations");
            return false;
          }
          int vnet = vnetIface->getVnet();
          printInfo("VNET: " + std::to_string(vnet));
        } else if (property == "mac") {
          std::string mac = iface->getMacAddress();
          printInfo("MAC: " + mac);
        } else {
          printError("Unknown property: " + property);
          return false;
        }
      } else {
        // Show comprehensive interface summary
        // Get interface type from interface object
        auto interfaceType = iface->getType();
        std::string type_str;
        switch (interfaceType) {
        case libfreebsdnet::interface::InterfaceType::ETHERNET:
          type_str = "Ethernet";
          break;
        case libfreebsdnet::interface::InterfaceType::LOOPBACK:
          type_str = "Loopback";
          break;
        case libfreebsdnet::interface::InterfaceType::BRIDGE:
          type_str = "Bridge";
          break;
        case libfreebsdnet::interface::InterfaceType::WIRELESS:
          type_str = "IEEE80211";
          break;
        case libfreebsdnet::interface::InterfaceType::L2VLAN:
          type_str = "L2VLAN";
          break;
        case libfreebsdnet::interface::InterfaceType::EPAIR:
          type_str = "EthernetPair";
          break;
        case libfreebsdnet::interface::InterfaceType::LAGG:
          type_str = "LinkAggregate";
          break;
        case libfreebsdnet::interface::InterfaceType::GIF:
          type_str = "GenericTunnel";
          break;
        default:
          type_str = "Unknown";
          break;
        }

        std::string status = iface->isUp() ? "UP" : "DOWN";
        auto flags = iface->getFlags();
        std::string flags_str = "";
        for (const auto &flag : flags) {
          switch (flag) {
          case libfreebsdnet::interface::Flag::UP:
            flags_str += "UP ";
            break;
          case libfreebsdnet::interface::Flag::RUNNING:
            flags_str += "RUNNING ";
            break;
          case libfreebsdnet::interface::Flag::BROADCAST:
            flags_str += "BROADCAST ";
            break;
          case libfreebsdnet::interface::Flag::MULTICAST:
            flags_str += "MULTICAST ";
            break;
          case libfreebsdnet::interface::Flag::LOOPBACK:
            flags_str += "LOOPBACK ";
            break;
          case libfreebsdnet::interface::Flag::POINTOPOINT:
            flags_str += "POINTOPOINT ";
            break;
          default:
            break;
          }
        }

        printInfo("Interface: " + name);
        printInfo("  Index:        " + std::to_string(iface->getIndex()));
        printInfo("  Type:         " + type_str);
        printInfo("  MTU:          " + std::to_string(iface->getMtu()));
        printInfo("  Status:       " + status);
        printInfo("  Flags:        " +
                  (flags_str.empty() ? "None" : flags_str));
        printInfo("  FIB:          " + std::to_string(iface->getFib()));
        printInfo("  Media:        0x" + std::to_string(iface->getMedia()));
        printInfo("  Capabilities: 0x" +
                  std::to_string(iface->getCapabilities()));
        auto vnetIface =
            dynamic_cast<libfreebsdnet::interface::VnetInterface *>(
                iface.get());
        if (vnetIface) {
          std::string jailName = vnetIface->getVnetJailName();
          if (!jailName.empty()) {
            int vnetId = vnetIface->getVnet();
            printInfo("  VNET:         " + jailName +
                      " (jid: " + std::to_string(vnetId) + ")");
          }
          // If jail name is empty, don't display VNET field at all
        }
        // If not a VnetInterface, don't display VNET field at all
        printInfo("  MAC:          " + iface->getMacAddress());

        auto groups = iface->getGroups();
        if (!groups.empty()) {
          std::string groupList;
          for (size_t i = 0; i < groups.size(); i++) {
            if (i > 0)
              groupList += ", ";
            groupList += groups[i];
          }
          printInfo("  Groups:        " + groupList);
        } else {
          printInfo("  Groups:        None");
        }

        auto addresses = iface->getAddresses();
        if (!addresses.empty()) {
          printInfo("  Addresses:");
          for (const auto &addr : addresses) {
            printInfo("    " + addr.getCidr());
          }
        } else {
          printInfo("  Addresses:     None");
        }

        // Show additional interface-specific information
        if (type_str == "Bridge") {
          auto bridgeIface =
              dynamic_cast<libfreebsdnet::interface::BridgeInterface *>(
                  iface.get());
          if (bridgeIface) {
            printInfo("  Bridge Info:");
            printInfo("    STP:          " +
                      std::string(bridgeIface->isStpEnabled() ? "ON" : "OFF"));
            int aging = bridgeIface->getAgingTime();
            if (aging > 0) {
              printInfo("    Ageing:       " + std::to_string(aging) + "s");
            }
            int hello = bridgeIface->getHelloTime();
            if (hello > 0) {
              printInfo("    Hello Time:   " + std::to_string(hello) + "s");
            }
            int fwd = bridgeIface->getForwardDelay();
            if (fwd > 0) {
              printInfo("    Forward Delay:" + std::to_string(fwd) + "s");
            }
            int proto = bridgeIface->getProtocol();
            if (proto >= 0) {
              std::string protoStr = (proto == 0)   ? "STP"
                                     : (proto == 2) ? "RSTP"
                                                    : "Unknown";
              printInfo("    Protocol:     " + protoStr);
            }
            int maxAddr = bridgeIface->getMaxAddresses();
            if (maxAddr > 0) {
              printInfo("    Max Addresses:" + std::to_string(maxAddr));
            }
            int priority = bridgeIface->getPriority();
            if (priority >= 0) {
              printInfo("    Priority:     " + std::to_string(priority));
            }
            int rootCost = bridgeIface->getRootPathCost();
            if (rootCost >= 0) {
              printInfo("    Root Cost:    " + std::to_string(rootCost));
            }
          }
        } else if (type_str == "GenericTunnel") {
          auto gifIface =
              dynamic_cast<libfreebsdnet::interface::GifInterface *>(
                  iface.get());
          if (gifIface) {
            printInfo("  GIF Info:");
            std::string localAddr = gifIface->getLocalAddress();
            std::string remoteAddr = gifIface->getRemoteAddress();
            printInfo("    Local:        " +
                      (localAddr.empty() ? "None" : localAddr));
            printInfo("    Remote:       " +
                      (remoteAddr.empty() ? "None" : remoteAddr));
            int protocol = gifIface->getProtocol();
            printInfo("    Protocol:     " + std::to_string(protocol));
            int ttl = gifIface->getTtl();
            printInfo("    TTL:          " + std::to_string(ttl));
            bool pmtu = gifIface->isPmtuDiscoveryEnabled();
            printInfo("    PMTU Discovery:" + std::string(pmtu ? "ON" : "OFF"));
            int tunnelFib = gifIface->getTunnelFib();
            if (tunnelFib >= 0) {
              printInfo("    Tunnel FIB:   " + std::to_string(tunnelFib));
            }
          }
        } else if (type_str == "IEEE80211") {
          printInfo("  Wireless Info:");
          printInfo("    SSID: (wireless interface detected)");
          printInfo("    Mode: (wireless interface detected)");
          printInfo("    Channel: (wireless interface detected)");
          printInfo("    Signal: (wireless interface detected)");
        }
      }

      return true;
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleShowInterfaceType(const std::vector<std::string> &args) {
    if (args.size() < 4) {
      printError("Usage: show interface type <type>");
      return false;
    }

    try {
      std::string type = args[3];

      if (type == "bridge") {
        return handleShowInterfaceTypeBridge(args);
      } else if (type == "lagg") {
        return handleShowInterfaceTypeLagg(args);
      } else if (type == "gif") {
        return handleShowInterfaceTypeGif(args);
      } else if (type == "ethernet") {
        return handleShowInterfaceTypeEthernet(args);
      } else {
        printError("Unsupported interface type: " + type);
        return false;
      }

    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleShowSystem(const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning

    try {
      printInfo("System Network Configuration");
      printInfo("============================");

      libfreebsdnet::system::SystemConfig config;

      // FIB Configuration
      printInfo("FIB Configuration:");
      printInfo("  net.fibs: " + std::to_string(config.getFibs()));
      printInfo("  net.add_addr_allfibs: " +
                std::string(config.getAddAddrAllFibs() ? "1" : "0"));

      // IP Forwarding
      printInfo("\nIP Forwarding:");
      printInfo("  IPv4 forwarding: " +
                std::string(config.getIpForwarding() ? "1" : "0"));
      printInfo("  IPv6 forwarding: " +
                std::string(config.getIp6Forwarding() ? "1" : "0"));

      // Route Configuration
      printInfo("\nRoute Configuration:");
      printInfo("  net.route.multipath: " +
                std::string(config.getRouteMultipath() ? "1" : "0"));
      printInfo("  net.route.hash_outbound: " +
                std::string(config.getRouteHashOutbound() ? "1" : "0"));
      printInfo("  net.route.ipv6_nexthop: " +
                std::string(config.getRouteIpv6Nexthop() ? "1" : "0"));

      // Route Algorithms
      printInfo("\nRoute Algorithms:");
      printInfo("  IPv4 algorithm: " + config.getRouteInetAlgo());
      printInfo("  IPv6 algorithm: " + config.getRouteInet6Algo());

      // Performance Settings
      printInfo("\nPerformance Settings:");
      printInfo("  NetISR max queue length: " +
                std::to_string(config.getNetisrMaxqlen()));
      printInfo("  FIB max sync delay: " +
                std::to_string(config.getFibMaxSyncDelay()) + " ms");

      return true;

    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

} // namespace net
