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

} // namespace net
