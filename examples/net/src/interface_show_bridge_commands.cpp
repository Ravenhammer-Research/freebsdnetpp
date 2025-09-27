/**
 * @file interface_show_bridge_commands.cpp
 * @brief Net tool interface show bridge command implementations
 * @details Implementation of bridge-specific show command handlers for the net
 * tool
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

  bool
  NetTool::handleShowInterfaceTypeBridge(const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning

    auto interfaces = interfaceManager.getInterfaces();

    // Filter for bridge interfaces
    std::vector<std::unique_ptr<libfreebsdnet::interface::Interface>>
        bridgeInterfaces;
    for (auto &interface : interfaces) {
      if (interface->getType() ==
          libfreebsdnet::interface::InterfaceType::BRIDGE) {
        bridgeInterfaces.push_back(std::move(interface));
      }
    }

    if (bridgeInterfaces.empty()) {
      printInfo("No bridge interfaces found.");
      return true;
    }

    printInfo("Bridge Interfaces");
    printInfo("=================");
    printInfo("");

    // Prepare table data
    std::vector<std::vector<std::string>> data;
    std::vector<std::string> headers = {"Interface", "Status", "FIB", "STP",
                                        "Members"};

    for (const auto &interface : bridgeInterfaces) {
      // Get interface information directly
      int fib = interface->getFib();
      std::vector<std::string> memberList;

      auto groups = interface->getGroups();
      if (!groups.empty()) {
        for (const auto &group : groups) {
          if (group != "all" && group != "bridge" && group != "member") {
            memberList.push_back(group);
          }
        }
      }

      // Get STP status
      std::string stpStatus = "Unknown";

      // Cast to BridgeInterface to access bridge-specific methods
      auto bridgeIface =
          dynamic_cast<libfreebsdnet::interface::BridgeInterface *>(
              interface.get());
      if (bridgeIface) {
        // Get actual STP status
        if (bridgeIface->isStpEnabled()) {
          stpStatus = "ON";
        } else {
          stpStatus = "OFF";
        }
      } else {
        stpStatus = "N/A";
      }

      // Create rows for bridge info with members on separate lines
      if (memberList.empty()) {
        std::vector<std::string> row;
        row.push_back(interface->getName());
        row.push_back(interface->isUp() ? "UP" : "DOWN");
        row.push_back(std::to_string(fib));
        row.push_back(stpStatus);
        row.push_back("None");
        data.push_back(row);
      } else {
        // First row with first member
        std::vector<std::string> row;
        row.push_back(interface->getName());
        row.push_back(interface->isUp() ? "UP" : "DOWN");
        row.push_back(std::to_string(fib));
        row.push_back(stpStatus);
        row.push_back(memberList[0]);
        data.push_back(row);

        // Additional rows for remaining members
        for (size_t i = 1; i < memberList.size(); i++) {
          std::vector<std::string> memberRow;
          memberRow.push_back("");
          memberRow.push_back("");
          memberRow.push_back("");
          memberRow.push_back("");
          memberRow.push_back(memberList[i]);
          data.push_back(memberRow);
        }
      }
    }

    printTable(data, headers);

    return true;
  }

} // namespace net
