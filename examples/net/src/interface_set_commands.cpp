/**
 * @file interface_set_commands.cpp
 * @brief Net tool interface set command implementations
 * @details Implementation of interface set-related command handlers for the net
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
        iface = interfaceManager.createInterface(
            name, 0, 0, interfaceType);

        if (!iface) {
          printError("Failed to create interface: " + name);
          return false;
        }

        printInfo("Created new interface: " + name);
      } else {
        // Interface exists, get the existing interface object
        iface = interfaceManager.getInterface(name);

        if (!iface) {
          printError("Failed to get interface object for: " + name);
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
      } else if (property == "port") {
        // Add interface as lagg port
        if (iface->getType() == libfreebsdnet::interface::InterfaceType::LAGG) {
          auto lagIface = dynamic_cast<libfreebsdnet::interface::LagInterface*>(iface.get());
          if (lagIface) {
            bool allSuccess = true;
            std::vector<std::string> addedPorts;
            std::vector<std::string> failedPorts;
            
            // Add all ports from args starting from index 4 (after 'port')
            for (size_t i = 4; i < args.size(); i++) {
              if (lagIface->addInterface(args[i])) {
                addedPorts.push_back(args[i]);
              } else {
                failedPorts.push_back(args[i]);
                allSuccess = false;
              }
            }
            
            if (allSuccess) {
              std::string portList = addedPorts[0];
              for (size_t i = 1; i < addedPorts.size(); i++) {
                portList += ", " + addedPorts[i];
              }
              printSuccess("Added ports [" + portList + "] to " + name);
              return true;
            } else {
              std::string errorMsg = "Failed to add some ports: ";
              for (const auto& port : failedPorts) {
                errorMsg += port + " ";
              }
              printError(errorMsg + lagIface->getLastError());
              return false;
            }
          } else {
            printError("Interface is not a LAGG interface");
            return false;
          }
        } else {
          printError("Port command only works with LAGG interfaces");
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
        // Set interface mode (lacp, etc.)
        if (name.substr(0, 4) == "lagg") {
          // For LAGG interfaces, set the protocol
          auto laggIface = dynamic_cast<libfreebsdnet::interface::LagInterface*>(iface.get());
          if (laggIface) {
            libfreebsdnet::interface::LagProtocol protocol;
            
            if (value == "lacp") {
              protocol = libfreebsdnet::interface::LagProtocol::LACP;
            } else if (value == "failover") {
              protocol = libfreebsdnet::interface::LagProtocol::FAILOVER;
            } else if (value == "loadbalance") {
              protocol = libfreebsdnet::interface::LagProtocol::LOADBALANCE;
            } else if (value == "roundrobin") {
              protocol = libfreebsdnet::interface::LagProtocol::ROUNDROBIN;
            } else if (value == "fec") {
              protocol = libfreebsdnet::interface::LagProtocol::FEC;
            } else {
              printError("Unknown LAGG protocol: " + value);
              return false;
            }
            
            if (laggIface->setProtocol(protocol)) {
              printSuccess("Set protocol " + value + " for LAGG interface " + name);
            } else {
              printError("Failed to set protocol " + value + " for LAGG interface " + name + ": " + laggIface->getLastError());
              return false;
            }
          } else {
            printError("Interface " + name + " is not a LAGG interface");
            return false;
          }
        } else {
          printError("Mode setting only supported for LAGG interfaces");
          return false;
        }
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
        // Set VNET - check if interface supports VNET
        auto vnetIface = dynamic_cast<libfreebsdnet::interface::VnetInterface*>(iface.get());
        if (!vnetIface) {
          printError("Interface " + name + " does not support VNET operations");
          return false;
        }
        int vnet = std::stoi(value);
        if (vnetIface->setVnet(vnet)) {
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
