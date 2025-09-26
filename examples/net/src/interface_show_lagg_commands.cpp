/**
 * @file interface_show_lagg_commands.cpp
 * @brief Net tool interface show lagg command implementations
 * @details Implementation of lagg-specific show command handlers for the net
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

  bool NetTool::handleShowInterfaceTypeLagg(const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning
    
    auto interfaces = interfaceManager.getInterfaces();
    
    // Filter for lagg interfaces (by name since IFT_LAGG may not be available)
    std::vector<std::unique_ptr<libfreebsdnet::interface::Interface>> laggInterfaces;
    for (auto& interface : interfaces) {
      if (interface->getName().substr(0, 4) == "lagg") {
        laggInterfaces.push_back(std::move(interface));
      }
    }
    
    if (laggInterfaces.empty()) {
      printInfo("No lagg interfaces found.");
      return true;
    }
    
    printInfo("LAGG Interfaces");
    printInfo("===============");
    printInfo("");
    
    // Print table header
    printf("%-12s %-6s %-8s %-6s %-8s %-12s %-8s %-20s\n", 
           "Interface", "Index", "Status", "MTU", "FIB", "Protocol", "Hash", "Ports");
    printf("%-12s %-6s %-8s %-6s %-8s %-12s %-8s %-20s\n", 
           "----------", "-----", "------", "---", "---", "----------", "----", "-----");
    
    for (const auto& interface : laggInterfaces) {
      // Get interface information directly
      int fib = interface->getFib();
      std::vector<std::string> portList;
      std::string protocol = "Unknown";
      std::string hash = "Unknown";
      
      // Cast to LagInterface to get lagg-specific information
      auto laggIface = dynamic_cast<libfreebsdnet::interface::LagInterface*>(interface.get());
      if (laggIface) {
        // Get ports from LagInterface
        portList = laggIface->getPorts();
        
        // Get protocol
          auto proto = laggIface->getProtocol();
          switch (proto) {
            case libfreebsdnet::interface::LagProtocol::FAILOVER:
              protocol = "failover";
              break;
            case libfreebsdnet::interface::LagProtocol::FEC:
              protocol = "fec";
              break;
            case libfreebsdnet::interface::LagProtocol::LACP:
              protocol = "lacp";
              break;
            case libfreebsdnet::interface::LagProtocol::LOADBALANCE:
              protocol = "loadbalance";
              break;
            case libfreebsdnet::interface::LagProtocol::ROUNDROBIN:
              protocol = "roundrobin";
              break;
            default:
              protocol = "unknown";
              break;
          }
          
          // Get hash type
          hash = laggIface->getHashType();
        } else {
          // Fallback to groups if not a LagInterface
          auto groups = interface->getGroups();
          if (!groups.empty()) {
            for (const auto& group : groups) {
              if (group != "all" && group != "lagg") {
                portList.push_back(group);
              }
            }
          }
          protocol = "Unknown";
          hash = "Unknown";
        }
      
      // Split hash by commas
      std::vector<std::string> hashList;
      if (!hash.empty() && hash != "Unknown") {
        std::stringstream hashStream(hash);
        std::string hashItem;
        while (std::getline(hashStream, hashItem, ',')) {
          // Trim whitespace
          hashItem.erase(0, hashItem.find_first_not_of(" \t"));
          hashItem.erase(hashItem.find_last_not_of(" \t") + 1);
          if (!hashItem.empty()) {
            hashList.push_back(hashItem);
          }
        }
      }
      
      // Format ports list
      std::string portsStr = "None";
      if (!portList.empty()) {
        portsStr = portList[0];
        for (size_t i = 1; i < portList.size(); i++) {
          portsStr += "," + portList[i];
        }
      }
      
      // Split ports by commas
      std::vector<std::string> portStrList;
      if (!portList.empty()) {
        portStrList = portList;
      } else {
        portStrList.push_back("None");
      }
      
      // Determine the maximum number of rows needed
      size_t maxRows = std::max(hashList.size(), portStrList.size());
      if (maxRows == 0) {
        maxRows = 1; // At least one row
      }
      
      // Print lagg info with multiple rows for hash and ports
      for (size_t i = 0; i < maxRows; i++) {
        std::string hashValue = (i < hashList.size()) ? hashList[i] : "";
        std::string portValue = (i < portStrList.size()) ? portStrList[i] : "";
        
        if (i == 0) {
          // First row includes all other columns
        printf("%-12s %-6u %-8s %-6d %-8d %-12s %-8s %-20s\n",
               interface->getName().c_str(),
               interface->getIndex(),
               interface->isUp() ? "UP" : "DOWN",
               interface->getMtu(),
               fib,
               protocol.c_str(),
               hashValue.c_str(),
               portValue.c_str());
        } else {
          // Subsequent rows only show hash and ports, with spaces for other columns
          printf("%-12s %-6s %-8s %-6s %-8s %-12s %-8s %-20s\n",
                 "",
                 "",
                 "",
                 "",
                 "",
                 "",
                 hashValue.c_str(),
                 portValue.c_str());
        }
      }
    }
    
    return true;
  }

} // namespace net
