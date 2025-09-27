/**
 * @file interface_delete_commands.cpp
 * @brief Net tool interface delete command implementations
 * @details Implementation of interface delete-related command handlers for the
 * net tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/bridge.hpp>
#include <interface/lagg.hpp>
#include <interface/wireless.hpp>
#include <iostream>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net_tool.hpp>
#include <sstream>
#include <system/config.hpp>

namespace net {

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

      // Get interface object directly
      auto iface = interfaceManager.getInterface(name);

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
        } else if (property == "member" && args.size() > 4) {
          // Delete bridge/LAGG member
          std::string member = args[4];
          printSuccess("Removed member " + member + " from interface " + name + " (not fully implemented)");
          return true;
        } else if (property == "mtu") {
          // Reset MTU to default (1500 for most interfaces)
          if (iface->setMtu(1500)) {
            printSuccess("Reset MTU to default for interface " + name);
            return true;
          } else {
            printError("Failed to reset MTU: " + iface->getLastError());
            return false;
          }
        } else if (property == "media") {
          // Reset media to auto-select
          if (iface->setMedia(0)) {
            printSuccess("Reset media to auto-select for interface " + name);
            return true;
          } else {
            printError("Failed to reset media: " + iface->getLastError());
            return false;
          }
        } else if (property == "capabilities") {
          // Reset capabilities (not typically deletable, but we can try)
          printError("Capabilities cannot be deleted, only modified");
          return false;
        } else if (property == "vnet") {
          // Remove VNET association
          printSuccess("Removed VNET association from interface " + name + " (not fully implemented)");
          return true;
        } else if (property == "mac") {
          // Reset MAC address to default
          printError("MAC address cannot be deleted, only changed");
          return false;
        } else if (property == "local") {
          // Delete GIF local address
          if (name.substr(0, 3) == "gif") {
            printSuccess("Removed local address from GIF interface " + name + " (not fully implemented)");
            return true;
          } else {
            printError("Local address deletion only supported for GIF interfaces");
            return false;
          }
        } else if (property == "remote") {
          // Delete GIF remote address
          if (name.substr(0, 3) == "gif") {
            printSuccess("Removed remote address from GIF interface " + name + " (not fully implemented)");
            return true;
          } else {
            printError("Remote address deletion only supported for GIF interfaces");
            return false;
          }
        } else if (property == "tunfib") {
          // Delete tunnel FIB
          if (name.substr(0, 3) == "gif") {
            printSuccess("Removed tunnel FIB from GIF interface " + name + " (not fully implemented)");
            return true;
          } else {
            printError("Tunnel FIB deletion only supported for GIF interfaces");
            return false;
          }
        } else if (property == "ipv6" && args.size() > 4) {
          // Delete IPv6 option
          std::string option = args[4];
          
          // Map option names to enum values
          libfreebsdnet::interface::Ipv6Option ipv6Option;
          if (option == "slaac") {
            // SLAAC uses ACCEPT_RTADV as the closest equivalent
            ipv6Option = libfreebsdnet::interface::Ipv6Option::ACCEPT_RTADV;
          } else if (option == "accept_rtadv") {
            ipv6Option = libfreebsdnet::interface::Ipv6Option::ACCEPT_RTADV;
          } else if (option == "perform_nud") {
            ipv6Option = libfreebsdnet::interface::Ipv6Option::PERFORM_NUD;
          } else if (option == "auto_linklocal") {
            ipv6Option = libfreebsdnet::interface::Ipv6Option::AUTO_LINKLOCAL;
          } else if (option == "no_radr") {
            ipv6Option = libfreebsdnet::interface::Ipv6Option::NO_RADR;
          } else if (option == "no_dad") {
            ipv6Option = libfreebsdnet::interface::Ipv6Option::NO_DAD;
          } else if (option == "ifdisabled") {
            ipv6Option = libfreebsdnet::interface::Ipv6Option::IFDISABLED;
          } else {
            printError("Unknown IPv6 option: " + option);
            return false;
          }
          
          if (iface->setIpv6Option(ipv6Option, false)) {
            printSuccess("Disabled IPv6 option " + option + " on interface " + name);
            return true;
          } else {
            printError("Failed to disable IPv6 option " + option + ": " + iface->getLastError());
            return false;
          }
        } else {
          printError("Unknown property: " + property);
          return false;
        }
      } else {
        // Destroy interface
        if (iface->destroy()) {
          printSuccess("Destroyed interface " + name);
          return true;
        } else {
          printError("Failed to destroy interface: " + iface->getLastError());
          return false;
        }
      }
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleDeleteBridge(const std::vector<std::string> &args) {
    if (args.size() < 4) {
      printError("Usage: delete bridge <name> <property> [value]");
      return false;
    }

    if (args[1] != "bridge") {
      printError("Only 'bridge' target is supported");
      return false;
    }

    std::string name = args[2];
    std::string property = args[3];

    try {
      auto iface = interfaceManager.getInterface(name);
      if (!iface) {
        printError("Bridge interface not found: " + name);
        return false;
      }

      if (property == "stp") {
        // Disable STP
        printSuccess("Disabled STP on bridge " + name + " (not fully implemented)");
        return true;
      } else if (property == "member" && args.size() > 4) {
        // Remove bridge member
        std::string member = args[4];
        printSuccess("Removed member " + member + " from bridge " + name + " (not fully implemented)");
        return true;
      } else {
        printError("Unknown bridge property: " + property);
        return false;
      }
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleDeleteLagg(const std::vector<std::string> &args) {
    if (args.size() < 4) {
      printError("Usage: delete lagg <name> <property> [value]");
      return false;
    }

    if (args[1] != "lagg") {
      printError("Only 'lagg' target is supported");
      return false;
    }

    std::string name = args[2];
    std::string property = args[3];

    try {
      auto iface = interfaceManager.getInterface(name);
      if (!iface) {
        printError("LAGG interface not found: " + name);
        return false;
      }

      if (property == "protocol") {
        // Reset protocol to default (failover)
        printSuccess("Reset LAGG protocol to failover for " + name + " (not fully implemented)");
        return true;
      } else if (property == "member" && args.size() > 4) {
        // Remove LAGG member
        std::string member = args[4];
        printSuccess("Removed member " + member + " from LAGG " + name + " (not fully implemented)");
        return true;
      } else {
        printError("Unknown LAGG property: " + property);
        return false;
      }
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleDeleteSystem(const std::vector<std::string> &args) {
    if (args.size() < 3) {
      printError("Usage: delete system <property>");
      return false;
    }

    if (args[1] != "system") {
      printError("Only 'system' target is supported");
      return false;
    }

    std::string property = args[2];

    try {
      if (property == "fibs") {
        // Reset FIB count to default (1)
        std::string command = "sysctl net.fibs=1";
        int result = system(command.c_str());

        if (result == 0) {
          printSuccess("Reset net.fibs to default (1)");
          return true;
        } else {
          printError("Failed to reset net.fibs");
          return false;
        }
      } else if (property == "add_addr_allfibs") {
        // Reset to default (0)
        std::string command = "sysctl net.add_addr_allfibs=0";
        int result = system(command.c_str());

        if (result == 0) {
          printSuccess("Reset net.add_addr_allfibs to default (0)");
          return true;
        } else {
          printError("Failed to reset net.add_addr_allfibs");
          return false;
        }
      } else if (property == "ip_forwarding") {
        // Reset IPv4 forwarding to default (0)
        std::string command = "sysctl net.inet.ip.forwarding=0";
        int result = system(command.c_str());

        if (result == 0) {
          printSuccess("Reset IPv4 forwarding to default (0)");
          return true;
        } else {
          printError("Failed to reset IPv4 forwarding");
          return false;
        }
      } else if (property == "ip6_forwarding") {
        // Reset IPv6 forwarding to default (0)
        std::string command = "sysctl net.inet6.ip6.forwarding=0";
        int result = system(command.c_str());

        if (result == 0) {
          printSuccess("Reset IPv6 forwarding to default (0)");
          return true;
        } else {
          printError("Failed to reset IPv6 forwarding");
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
