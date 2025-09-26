/**
 * @file interface_show_gif_commands.cpp
 * @brief Net tool GIF interface show command implementations
 * @details Implementation of GIF interface show-related command handlers for the net
 * tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/gif.hpp>
#include <system/config.hpp>
#include <iostream>
#include <sstream>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net_tool.hpp>

namespace net {

  bool NetTool::handleShowInterfaceTypeGif(const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning
    
    auto interfaces = interfaceManager.getInterfaces();
    
    // Filter for GIF interfaces (by name since IFT_GIF may not be available)
    std::vector<std::unique_ptr<libfreebsdnet::interface::Interface>> gifInterfaces;
    for (auto& interface : interfaces) {
      if (interface->getName().substr(0, 3) == "gif") {
        gifInterfaces.push_back(std::move(interface));
      }
    }
    
    if (gifInterfaces.empty()) {
      printInfo("No GIF interfaces found.");
      return true;
    }
    
    printInfo("GIF Interfaces");
    printInfo("==============");
    printInfo("");
    
    // Prepare table data
    std::vector<std::vector<std::string>> data;
    std::vector<std::string> headers = {"Interface", "Status", "Local", "Remote", "FIB"};
    
    for (const auto& interface : gifInterfaces) {
      auto gifIface = dynamic_cast<libfreebsdnet::interface::GifInterface*>(interface.get());
      if (!gifIface) {
        continue;
      }
      
      std::vector<std::string> row;
      row.push_back(interface->getName());
      row.push_back(interface->isUp() ? "UP" : "DOWN");
      
      std::string localAddr = gifIface->getLocalAddress();
      std::string remoteAddr = gifIface->getRemoteAddress();
      if (localAddr.empty()) localAddr = "None";
      if (remoteAddr.empty()) remoteAddr = "None";
      
      row.push_back(localAddr);
      row.push_back(remoteAddr);
      row.push_back(std::to_string(interface->getFib()));
      
      data.push_back(row);
    }
    
    printTable(data, headers);
    return true;
  }

} // namespace net
