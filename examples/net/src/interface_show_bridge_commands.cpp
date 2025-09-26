/**
 * @file interface_show_bridge_commands.cpp
 * @brief Net tool interface show bridge command implementations
 * @details Implementation of bridge-specific show command handlers for the net
 * tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/wireless.hpp>
#include <interface/bridge.hpp>
#include <interface/lagg.hpp>
#include <system/config.hpp>
#include <iostream>
#include <sstream>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net_tool.hpp>

namespace net {

  bool NetTool::handleShowInterfaceTypeBridge(const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning
    
    auto interfaces = interfaceManager.getInterfaces();
    
    // Filter for bridge interfaces
    std::vector<std::unique_ptr<libfreebsdnet::interface::Interface>> bridgeInterfaces;
    for (auto& interface : interfaces) {
      if (interface->getType() == libfreebsdnet::interface::InterfaceType::BRIDGE) {
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
    
    // Print table header
    printf("%-12s %-6s %-8s %-6s %-8s %-6s %-8s %-20s\n", 
           "Interface", "Index", "Status", "MTU", "FIB", "STP", "Ageing", "Members");
    printf("%-12s %-6s %-8s %-6s %-8s %-6s %-8s %-20s\n", 
           "----------", "-----", "------", "---", "---", "---", "-------", "-------");
    
    for (const auto& interface : bridgeInterfaces) {
      // Get interface information directly
      int fib = interface->getFib();
      std::vector<std::string> memberList;
      
      auto groups = interface->getGroups();
      if (!groups.empty()) {
        for (const auto& group : groups) {
          if (group != "all" && group != "bridge" && group != "member") {
            memberList.push_back(group);
          }
        }
      }
      
      // Get additional bridge information
      std::string stpStatus = "Unknown";
      std::string ageingTime = "Unknown";
      
      // Cast to BridgeInterface to access bridge-specific methods
      auto bridgeIface = dynamic_cast<libfreebsdnet::interface::BridgeInterface*>(interface.get());
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
      
      // Print bridge info with members on separate lines
      if (memberList.empty()) {
        printf("%-12s %-6u %-8s %-6d %-8d %-6s %-8s %-20s\n",
               interface->getName().c_str(),
               interface->getIndex(),
               interface->isUp() ? "UP" : "DOWN",
               interface->getMtu(),
               fib,
               stpStatus.c_str(),
               ageingTime.c_str(),
               "None");
      } else {
        // Print first row with first member
        printf("%-12s %-6u %-8s %-6d %-8d %-6s %-8s %-20s\n",
               interface->getName().c_str(),
               interface->getIndex(),
               interface->isUp() ? "UP" : "DOWN",
               interface->getMtu(),
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
  }

} // namespace net
