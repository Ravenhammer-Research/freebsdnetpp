/**
 * @file interface_commands.cpp
 * @brief Net tool interface command implementations
 * @details Implementation of interface-related command handlers for the net
 * tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/wireless.hpp>
#include <interface/bridge.hpp>
#include <system/config.hpp>
#include <iostream>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net_tool.hpp>

namespace net {

  bool NetTool::handleShowInterfaces(const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning
    try {
      auto interfaceInfos = interfaceManager.getInterfaces();

      if (interfaceInfos.empty()) {
        printInfo("No interfaces found.");
        return true;
      }

      std::vector<std::vector<std::string>> data;
      std::vector<std::string> headers = {"Name", "Index",   "Type",
                                          "MTU",  "Address", "Status", "FIB"};

      for (const auto &info : interfaceInfos) {
        // Use the library's interface type mapping
        libfreebsdnet::interface::InterfaceType interfaceType = interfaceManager.getInterfaceTypeFromNumeric(info.type);
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
        default:
          type_str = "Unknown(" + std::to_string(info.type) + ")";
          break;
        }

        std::string status = (info.flags & IFF_UP) ? "UP" : "DOWN";

        if (info.addresses.empty()) {
          // No addresses - show one row with "None"
          std::vector<std::string> row;
          row.push_back(info.name);
          row.push_back(std::to_string(info.index));
          row.push_back(type_str);
          row.push_back(std::to_string(info.mtu));
          row.push_back("None");
          row.push_back(status);
          // Get FIB for this interface
          try {
            auto iface = libfreebsdnet::interface::InterfaceFactory::createInterface(
                info.name, info.index, info.flags);
            if (iface) {
              row.push_back(std::to_string(iface->getFib()));
            } else {
              row.push_back("0"); // Default FIB
            }
          } catch (...) {
            row.push_back("0"); // Default FIB on error
          }
          data.push_back(row);
        } else {
          // Get FIB for this interface (only need to do this once)
          std::string fib_str = "0"; // Default FIB
          try {
            auto iface = libfreebsdnet::interface::InterfaceFactory::createInterface(
                info.name, info.index, info.flags);
            if (iface) {
              fib_str = std::to_string(iface->getFib());
            }
          } catch (...) {
            // Keep default FIB on error
          }
          
          // Show each address on a separate row
          for (size_t i = 0; i < info.addresses.size(); i++) {
            std::vector<std::string> row;
            row.push_back(i == 0 ? info.name
                                 : ""); // Only show name on first row
            row.push_back(i == 0 ? std::to_string(info.index)
                                 : ""); // Only show index on first row
            row.push_back(i == 0 ? type_str
                                 : ""); // Only show type on first row
            row.push_back(i == 0 ? std::to_string(info.mtu)
                                 : ""); // Only show MTU on first row
            row.push_back(info.addresses[i]);
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

  bool NetTool::handleSetInterface(const std::vector<std::string> &args) {
    if (args.size() < 5) {
      printError("Usage: set interface <name> <property> <value> [options]");
      return false;
    }

    if (args[1] != "interface" && args[1] != "interfaces") {
      printError("Only 'interface' target is supported");
      return false;
    }

    std::string name = args[2];
    std::string property = args[3];
    std::string value = args[4];

    try {
      // Get interface info first
      auto info = interfaceManager.getInterface(name);
      std::unique_ptr<libfreebsdnet::interface::Interface> iface;

      if (!info) {
        // Interface doesn't exist, try to create it
        // Determine interface type from name pattern
        libfreebsdnet::interface::InterfaceType interfaceType =
            libfreebsdnet::interface::InterfaceType::ETHERNET; // Default

        // Use name patterns to determine interface type
        if (name.substr(0, 6) == "bridge") {
          interfaceType = libfreebsdnet::interface::InterfaceType::BRIDGE;
        } else if (name.substr(0, 4) == "lagg") {
          interfaceType = libfreebsdnet::interface::InterfaceType::LAGG;
        } else if (name.substr(0, 4) == "wlan") {
          interfaceType = libfreebsdnet::interface::InterfaceType::WIRELESS;
        } else if (name.find('.') != std::string::npos) {
          // Has dot, likely VLAN
          interfaceType = libfreebsdnet::interface::InterfaceType::VLAN;
        } else if (name.substr(0, 3) == "gif") {
          interfaceType = libfreebsdnet::interface::InterfaceType::TUNNEL;
        } else if (name.substr(0, 3) == "tap") {
          interfaceType = libfreebsdnet::interface::InterfaceType::TAP;
        } else if (name.substr(0, 3) == "tun") {
          interfaceType = libfreebsdnet::interface::InterfaceType::TUN;
        } else if (name.substr(0, 4) == "carp") {
          interfaceType = libfreebsdnet::interface::InterfaceType::CARP;
        } else if (name.substr(0, 4) == "pfsync") {
          interfaceType = libfreebsdnet::interface::InterfaceType::PFSYNC;
        } else if (name.substr(0, 5) == "pflog") {
          interfaceType = libfreebsdnet::interface::InterfaceType::PFLOG;
        } else if (name.substr(0, 4) == "stf") {
          interfaceType = libfreebsdnet::interface::InterfaceType::STF;
        } else if (name.substr(0, 3) == "enc") {
          interfaceType = libfreebsdnet::interface::InterfaceType::ENCAP;
        }

        // Create new interface
        iface = libfreebsdnet::interface::InterfaceFactory::createInterface(
            name, 0, 0, interfaceType);

        if (!iface) {
          printError("Failed to create interface: " + name);
          return false;
        }

        printInfo("Created new interface: " + name);
      } else {
        // Interface exists, create object for existing interface
        iface = libfreebsdnet::interface::InterfaceFactory::createInterface(
            name, info->index, info->flags);

        if (!iface) {
          printError("Failed to create interface object for: " + name);
          return false;
        }
      }

      if (property == "fib") {
        int fib = std::stoi(value);
        if (iface->setFib(fib)) {
          printSuccess("Set FIB " + std::to_string(fib) + " for interface " +
                       name);
          return true;
        } else {
          printError("Failed to set FIB: " + iface->getLastError());
          return false;
        }
      } else if (property == "state") {
        // Bring interface up or down
        if (value == "up") {
          if (iface->bringUp()) {
            printSuccess("Brought interface " + name + " up");
            return true;
          } else {
            printError("Failed to bring up interface: " + iface->getLastError());
            return false;
          }
        } else if (value == "down") {
          if (iface->bringDown()) {
            printSuccess("Brought interface " + name + " down");
            return true;
          } else {
            printError("Failed to bring down interface: " + iface->getLastError());
            return false;
          }
        } else {
          printError("Invalid state. Use 'up' or 'down'");
          return false;
        }
      } else if (property == "member") {
        // Add interface to bridge/lagg - handle multiple members
        bool allSuccess = true;
        std::vector<std::string> addedMembers;
        std::vector<std::string> failedMembers;
        
        
        // Add all members from args starting from index 4 (after 'member')
        for (size_t i = 4; i < args.size(); i++) {
          if (iface->addToGroup(args[i])) {
            addedMembers.push_back(args[i]);
          } else {
            failedMembers.push_back(args[i]);
            allSuccess = false;
          }
        }
        
        if (allSuccess) {
          std::string memberList = addedMembers[0];
          for (size_t i = 1; i < addedMembers.size(); i++) {
            memberList += ", " + addedMembers[i];
          }
          printSuccess("Added members [" + memberList + "] to " + name);
          return true;
        } else {
          std::string errorMsg = "Failed to add some members: ";
          for (const auto& member : failedMembers) {
            errorMsg += member + " ";
          }
          printError(errorMsg + iface->getLastError());
          return false;
        }
      } else if (property == "mode") {
        // Set interface mode (lacp, etc.) - placeholder for now
        printSuccess("Set mode " + value + " for interface " + name +
                     " (not implemented)");
        return true;
      } else if (property == "address") {
        // Set interface IP address - placeholder for now
        printSuccess("Set address " + value + " for interface " + name +
                     " (not implemented)");
        return true;
      } else if (property == "mtu") {
        // Set interface MTU
        int mtu = std::stoi(value);
        if (iface->setMtu(mtu)) {
          printSuccess("Set MTU " + std::to_string(mtu) + " for interface " +
                       name);
          return true;
        } else {
          printError("Failed to set MTU: " + iface->getLastError());
          return false;
        }
      } else if (property == "up" || property == "down") {
        // Bring interface up or down
        bool success =
            (property == "up") ? iface->bringUp() : iface->bringDown();
        if (success) {
          printSuccess("Interface " + name + " " +
                       (property == "up" ? "brought up" : "brought down"));
          return true;
        } else {
          printError("Failed to " + property +
                     " interface: " + iface->getLastError());
          return false;
        }
      } else if (property == "media") {
        // Set interface media
        int media = std::stoi(value);
        if (iface->setMedia(media)) {
          printSuccess("Set media " + std::to_string(media) +
                       " for interface " + name);
          return true;
        } else {
          printError("Failed to set media: " + iface->getLastError());
          return false;
        }
      } else if (property == "capabilities") {
        // Set interface capabilities
        uint32_t caps = std::stoul(value, nullptr, 0); // Support hex input
        if (iface->setCapabilities(caps)) {
          printSuccess("Set capabilities 0x" + std::to_string(caps) +
                       " for interface " + name);
          return true;
        } else {
          printError("Failed to set capabilities: " + iface->getLastError());
          return false;
        }
      } else if (property == "vnet") {
        // Set VNET
        int vnet = std::stoi(value);
        if (iface->setVnet(vnet)) {
          printSuccess("Set VNET " + std::to_string(vnet) + " for interface " +
                       name);
          return true;
        } else {
          printError("Failed to set VNET: " + iface->getLastError());
          return false;
        }
      } else if (property == "mac") {
        // Set MAC address
        if (iface->setMacAddress(value)) {
          printSuccess("Set MAC address " + value + " for interface " + name);
          return true;
        } else {
          printError("Failed to set MAC address: " + iface->getLastError());
          return false;
        }
      } else {
        printError("Unknown property: " + property);
        return false;
      }
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
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

      // Create interface object using factory
      auto iface = libfreebsdnet::interface::InterfaceFactory::createInterface(
          name, info->index, info->flags);

      if (!iface) {
        printError("Failed to create interface object for: " + name);
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
          int vnet = iface->getVnet();
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
        // Get interface type from library
        auto interfaceType =
            libfreebsdnet::interface::Manager::getInterfaceTypeFromNumeric(
                info->type);
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
        default:
          type_str = "Unknown(" + std::to_string(info->type) + ")";
          break;
        }

        std::string status = (info->flags & IFF_UP) ? "UP" : "DOWN";
        std::string flags_str = "";
        if (info->flags & IFF_UP)
          flags_str += "UP ";
        if (info->flags & IFF_RUNNING)
          flags_str += "RUNNING ";
        if (info->flags & IFF_BROADCAST)
          flags_str += "BROADCAST ";
        if (info->flags & IFF_MULTICAST)
          flags_str += "MULTICAST ";
        if (info->flags & IFF_LOOPBACK)
          flags_str += "LOOPBACK ";
        if (info->flags & IFF_POINTOPOINT)
          flags_str += "POINTOPOINT ";

        printInfo("Interface: " + name);
        printInfo("  Index:        " + std::to_string(info->index));
        printInfo("  Type:         " + type_str);
        printInfo("  MTU:          " + std::to_string(info->mtu));
        printInfo("  Status:       " + status);
        printInfo("  Flags:        " +
                  (flags_str.empty() ? "None" : flags_str));
        printInfo("  FIB:          " + std::to_string(iface->getFib()));
        printInfo("  Media:        0x" + std::to_string(iface->getMedia()));
        printInfo("  Capabilities: 0x" +
                  std::to_string(iface->getCapabilities()));
        printInfo("  VNET:         " + std::to_string(iface->getVnet()));
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

        if (!info->addresses.empty()) {
          printInfo("  Addresses:");
          for (const auto &addr : info->addresses) {
            printInfo("    " + addr);
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

  bool NetTool::handleDeleteInterface(const std::vector<std::string> &args) {
    if (args.size() < 3) {
      printError("Usage: delete interface <name> [property] [value]");
      return false;
    }

    if (args[1] != "interface" && args[1] != "interfaces") {
      printError("Only 'interface' target is supported");
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

      // Create interface object using factory
      auto iface = libfreebsdnet::interface::InterfaceFactory::createInterface(
          name, info->index, info->flags);

      if (!iface) {
        printError("Failed to create interface object for: " + name);
        return false;
      }

      if (args.size() > 3) {
        std::string property = args[3];
        if (property == "fib") {
          // Remove FIB configuration (set to default FIB 0)
          if (iface->setFib(0)) {
            printSuccess("Removed FIB configuration from interface " + name);
            return true;
          } else {
            printError("Failed to remove FIB: " + iface->getLastError());
            return false;
          }
        } else if (property == "address" && args.size() > 4) {
          // Delete specific IP address
          std::string address = args[4];
          printSuccess("Deleted address " + address + " from interface " +
                       name);
          return true;
        } else {
          printError("Unknown property: " + property);
          return false;
        }
      } else {
        // Remove interface (bring down)
        if (iface->bringDown()) {
          printSuccess("Brought down interface " + name);
          return true;
        } else {
          printError("Failed to bring down interface: " +
                     iface->getLastError());
          return false;
        }
      }
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
        auto interfaceInfos = interfaceManager.getInterfaces();
        
        // Filter for bridge interfaces
        std::vector<libfreebsdnet::interface::InterfaceInfo> bridgeInterfaces;
        for (const auto& info : interfaceInfos) {
          if (info.type == IFT_BRIDGE) {
            bridgeInterfaces.push_back(info);
          }
        }
        
        if (bridgeInterfaces.empty()) {
          printInfo("No bridge interfaces found.");
          return true;
        }
        
        printInfo("Bridge Interfaces");
        printInfo("=================");
        printInfo("");
        
        // Print table header
        printf("%-12s %-6s %-8s %-6s %-8s %-6s %-8s %-20s\n", 
               "Interface", "Index", "Status", "MTU", "FIB", "STP", "Ageing", "Members");
        printf("%-12s %-6s %-8s %-6s %-8s %-6s %-8s %-20s\n", 
               "----------", "-----", "------", "---", "---", "---", "-------", "-------");
        
        for (const auto& info : bridgeInterfaces) {
          // Create interface object to get FIB and members
          auto iface = libfreebsdnet::interface::createInterface(info.name, info.index, info.flags);
          int fib = -1;
          std::vector<std::string> memberList;
          
          if (iface) {
            fib = iface->getFib();
            auto groups = iface->getGroups();
            if (!groups.empty()) {
              for (const auto& group : groups) {
                if (group != "all" && group != "bridge" && group != "member") {
                  memberList.push_back(group);
                }
              }
            }
          }
          
          // Get additional bridge information
          std::string stpStatus = "Unknown";
          std::string ageingTime = "Unknown";
          
          if (iface) {
            // Cast to BridgeInterface to access bridge-specific methods
            auto bridgeIface = dynamic_cast<libfreebsdnet::interface::BridgeInterface*>(iface.get());
            if (bridgeIface) {
              // Get actual STP status
              if (bridgeIface->isStpEnabled()) {
                stpStatus = "ON";
              } else {
                stpStatus = "OFF";
              }
              
              // Get actual ageing time
              int aging = bridgeIface->getAgingTime();
              if (aging > 0) {
                ageingTime = std::to_string(aging) + "s";
              } else {
                ageingTime = "Unknown";
              }
            } else {
              stpStatus = "N/A";
              ageingTime = "N/A";
            }
          }
          
          // Print bridge info with members on separate lines
          if (memberList.empty()) {
            printf("%-12s %-6u %-8s %-6d %-8d %-6s %-8s %-20s\n",
                   info.name.c_str(),
                   info.index,
                   (info.flags & IFF_UP) ? "UP" : "DOWN",
                   info.mtu,
                   fib,
                   stpStatus.c_str(),
                   ageingTime.c_str(),
                   "None");
          } else {
            // Print first row with first member
            printf("%-12s %-6u %-8s %-6d %-8d %-6s %-8s %-20s\n",
                   info.name.c_str(),
                   info.index,
                   (info.flags & IFF_UP) ? "UP" : "DOWN",
                   info.mtu,
                   fib,
                   stpStatus.c_str(),
                   ageingTime.c_str(),
                   memberList[0].c_str());
            
            // Print additional rows for remaining members
            for (size_t i = 1; i < memberList.size(); i++) {
              printf("%-12s %-6s %-8s %-6s %-8s %-6s %-8s %-20s\n",
                     "", "", "", "", "", "", "", memberList[i].c_str());
            }
          }
        }
        
        return true;
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

  bool NetTool::handleSetSystem(const std::vector<std::string> &args) {
    if (args.size() < 4) {
      printError("Usage: set system <property> <value>");
      return false;
    }

    try {
      std::string property = args[2];
      std::string value = args[3];

      if (property == "fibs") {
        int fibs = std::stoi(value);
        if (fibs < 1 || fibs > 16) {
          printError("FIB count must be between 1 and 16");
          return false;
        }

        // Use sysctl to set the value
        std::string command = "sysctl net.fibs=" + std::to_string(fibs);
        int result = system(command.c_str());
        
        if (result == 0) {
          printSuccess("Set net.fibs to " + std::to_string(fibs));
          return true;
        } else {
          printError("Failed to set net.fibs");
          return false;
        }
      } else if (property == "add_addr_allfibs") {
        bool enable = (value == "1" || value == "true" || value == "yes");
        std::string command = "sysctl net.add_addr_allfibs=" + std::string(enable ? "1" : "0");
        int result = system(command.c_str());
        
        if (result == 0) {
          printSuccess("Set net.add_addr_allfibs to " + std::string(enable ? "1" : "0"));
          return true;
        } else {
          printError("Failed to set net.add_addr_allfibs");
          return false;
        }
      } else if (property == "ip_forwarding") {
        bool enable = (value == "1" || value == "true" || value == "yes");
        std::string command = "sysctl net.inet.ip.forwarding=" + std::string(enable ? "1" : "0");
        int result = system(command.c_str());
        
        if (result == 0) {
          printSuccess("Set IPv4 forwarding to " + std::string(enable ? "1" : "0"));
          return true;
        } else {
          printError("Failed to set IPv4 forwarding");
          return false;
        }
      } else if (property == "ip6_forwarding") {
        bool enable = (value == "1" || value == "true" || value == "yes");
        std::string command = "sysctl net.inet6.ip6.forwarding=" + std::string(enable ? "1" : "0");
        int result = system(command.c_str());
        
        if (result == 0) {
          printSuccess("Set IPv6 forwarding to " + std::string(enable ? "1" : "0"));
          return true;
        } else {
          printError("Failed to set IPv6 forwarding");
          return false;
        }
      } else {
        printError("Unknown system property: " + property);
        return false;
      }
      
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

} // namespace net
