/**
 * @file interface_show_commands.cpp
 * @brief Net tool interface show command implementations
 * @details Implementation of interface show-related command handlers for the net
 * tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/wireless.hpp>
#include <interface/bridge.hpp>
#include <interface/lagg.hpp>
#include <interface/vnet.hpp>
#include <system/config.hpp>
#include <iostream>
#include <sstream>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net_tool.hpp>

namespace net {

  bool NetTool::handleShowInterfaces(const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning
    try {
      auto interfaces = interfaceManager.getInterfaces();

      if (interfaces.empty()) {
        printInfo("No interfaces found.");
        return true;
      }

      std::vector<std::vector<std::string>> data;
      std::vector<std::string> headers = {"Name", "Index",   "Type",
                                          "MTU",  "Address", "Status", "FIB"};

      for (const auto &interface : interfaces) {
        // Get interface type from the interface object
        libfreebsdnet::interface::InterfaceType interfaceType = interface->getType();
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
          type_str = "LAGG";
          break;
        default:
          type_str = "Unknown";
          break;
        }

        std::string status = interface->isUp() ? "UP" : "DOWN";

        // Get addresses from the interface object
        auto addresses = interface->getAddresses();
        if (addresses.empty()) {
          // No addresses - show one row with "None"
          std::vector<std::string> row;
          row.push_back(interface->getName());
          row.push_back(std::to_string(interface->getIndex()));
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
            row.push_back(i == 0 ? std::to_string(interface->getIndex())
                                 : ""); // Only show index on first row
            row.push_back(i == 0 ? type_str
                                 : ""); // Only show type on first row
            row.push_back(i == 0 ? std::to_string(interface->getMtu())
                                 : ""); // Only show MTU on first row
            row.push_back(addresses[i].getCidr());
            row.push_back(i == 0 ? status
                                 : ""); // Only show status on first row
            row.push_back(i == 0 ? fib_str
                                 : ""); // Only show FIB on first row
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
          auto vnetIface = dynamic_cast<libfreebsdnet::interface::VnetInterface*>(iface.get());
          if (!vnetIface) {
            printError("Interface " + name + " does not support VNET operations");
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
          type_str = "LAGG";
          break;
        default:
          type_str = "Unknown";
          break;
        }

        std::string status = iface->isUp() ? "UP" : "DOWN";
        std::string flags_str = "";
        int flags = iface->getFlags();
        if (flags & IFF_UP)
          flags_str += "UP ";
        if (flags & IFF_RUNNING)
          flags_str += "RUNNING ";
        if (flags & IFF_BROADCAST)
          flags_str += "BROADCAST ";
        if (flags & IFF_MULTICAST)
          flags_str += "MULTICAST ";
        if (flags & IFF_LOOPBACK)
          flags_str += "LOOPBACK ";
        if (flags & IFF_POINTOPOINT)
          flags_str += "POINTOPOINT ";

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
        auto vnetIface = dynamic_cast<libfreebsdnet::interface::VnetInterface*>(iface.get());
        if (vnetIface) {
          printInfo("  VNET:         " + std::to_string(vnetIface->getVnet()));
        } else {
          printInfo("  VNET:         Not supported");
        }
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
        if (type_str == "IEEE80211") {
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
      printInfo("  net.add_addr_allfibs: " + std::string(config.getAddAddrAllFibs() ? "1" : "0"));
      
      // IP Forwarding
      printInfo("\nIP Forwarding:");
      printInfo("  IPv4 forwarding: " + std::string(config.getIpForwarding() ? "1" : "0"));
      printInfo("  IPv6 forwarding: " + std::string(config.getIp6Forwarding() ? "1" : "0"));
      
      // Route Configuration
      printInfo("\nRoute Configuration:");
      printInfo("  net.route.multipath: " + std::string(config.getRouteMultipath() ? "1" : "0"));
      printInfo("  net.route.hash_outbound: " + std::string(config.getRouteHashOutbound() ? "1" : "0"));
      printInfo("  net.route.ipv6_nexthop: " + std::string(config.getRouteIpv6Nexthop() ? "1" : "0"));
      
      // Route Algorithms
      printInfo("\nRoute Algorithms:");
      printInfo("  IPv4 algorithm: " + config.getRouteInetAlgo());
      printInfo("  IPv6 algorithm: " + config.getRouteInet6Algo());
      
      // Performance Settings
      printInfo("\nPerformance Settings:");
      printInfo("  NetISR max queue length: " + std::to_string(config.getNetisrMaxqlen()));
      printInfo("  FIB max sync delay: " + std::to_string(config.getFibMaxSyncDelay()) + " ms");
      
      return true;
      
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

} // namespace net
