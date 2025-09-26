/**
 * @file route_commands.cpp
 * @brief Net tool routing command implementations
 * @details Implementation of routing-related command handlers for the net tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <iostream>
#include <map>
#include <net_tool.hpp>

namespace net {

  bool NetTool::handleShowRoute(const std::vector<std::string> &args) {
    try {
      int fib = 0; // Default FIB

      // Check for FIB specification
      if (args.size() >= 4 && args[2] == "fib") {
        fib = std::stoi(args[3]);
      }

      auto entries = routingTable.getEntries(fib);

      if (entries.empty()) {
        printInfo("No routes found for FIB " + std::to_string(fib));
        return true;
      }

      std::vector<std::vector<std::string>> data;
      std::vector<std::string> headers = {"Destination", "Netmask", "Scope", "Gateway", "Flags", "Interface"};

      for (const auto &entry : entries) {
        std::vector<std::string> row;
        
        // Use CIDR notation for network routes, no CIDR for host routes
        std::string destination = entry->getDestination();
        std::string scope_interface = "";
        
        // Extract scope interface from IPv6 addresses
        if (destination.find('%') != std::string::npos) {
          size_t scope_pos = destination.find('%');
          scope_interface = destination.substr(scope_pos + 1);
          destination = destination.substr(0, scope_pos);
        }
        
        if (entry->isNetwork() && destination.find('/') == std::string::npos) {
          // Add default CIDR based on address family
          if (destination.find(':') != std::string::npos) {
            // IPv6 - determine prefix length based on route type
            if (destination == "::") {
              destination = "::/0"; // Default route
            } else if (destination.find("fe80::") == 0) {
              destination += "/64"; // Link-local
            } else if (destination.find("ff02::") == 0) {
              destination += "/16"; // Multicast
            } else {
              destination += "/128"; // Host route
            }
          } else {
            // IPv4 - determine prefix length based on route type
            if (destination == "0.0.0.0") {
              destination = "0.0.0.0/0"; // Default route
            } else if (destination.find("127.") == 0) {
              destination += "/32"; // Loopback
            } else if (destination.find("10.") == 0 || destination.find("192.168.") == 0) {
              destination += "/24"; // Private networks
            } else {
              destination += "/32"; // Host route
            }
          }
        }
        
        row.push_back(destination);
        row.push_back(entry->getNetmask());
        row.push_back(scope_interface);
        row.push_back(entry->getGateway());
        row.push_back(std::to_string(entry->getFlags()));
        row.push_back(entry->getInterface());
        data.push_back(row);
      }

      printTable(data, headers);
      return true;
    } catch (const std::exception &e) {
      printError("Failed to get routes: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleSetRoute(const std::vector<std::string> &args) {
    if (args.size() < 4) {
      printError("Usage: set route <destination> <gateway> [interface] [fib "
                 "<number>]");
      return false;
    }

    try {
      std::string destination = args[2];
      std::string gateway = args[3];
      std::string interface = "";
      int fib = 0;

      // Parse optional parameters
      for (size_t i = 4; i < args.size(); i++) {
        if (args[i] == "fib" && i + 1 < args.size()) {
          fib = std::stoi(args[++i]);
        } else if (interface.empty()) {
          interface = args[i];
        }
      }

      if (routingTable.addEntry(destination, gateway, interface, 0, fib)) {
        printSuccess("Added route " + destination + " via " + gateway +
                     (interface.empty() ? "" : " on " + interface) +
                     " to FIB " + std::to_string(fib));
        return true;
      } else {
        printError("Failed to add route");
        return false;
      }
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleAddRoute(const std::vector<std::string> &args) {
    if (args.size() < 4) {
      printError("Usage: add route <destination> <gateway> [interface] [fib "
                 "<number>]");
      return false;
    }

    try {
      std::string destination = args[2];
      std::string gateway = args[3];
      std::string interface = "";
      int fib = 0;

      // Parse optional parameters
      for (size_t i = 4; i < args.size(); i++) {
        if (args[i] == "fib" && i + 1 < args.size()) {
          fib = std::stoi(args[++i]);
        } else if (interface.empty()) {
          interface = args[i];
        }
      }

      if (routingTable.addEntry(destination, gateway, interface, 0, fib)) {
        printSuccess("Added route " + destination + " via " + gateway +
                     (interface.empty() ? "" : " on " + interface) +
                     " to FIB " + std::to_string(fib));
        return true;
      } else {
        printError("Failed to add route");
        return false;
      }
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleDeleteRoute(const std::vector<std::string> &args) {
    if (args.size() < 3) {
      printError("Usage: delete route <destination> [fib <number>]");
      return false;
    }

    try {
      std::string destination = args[2];
      int fib = 0;

      // Parse optional FIB parameter
      for (size_t i = 3; i < args.size(); i++) {
        if (args[i] == "fib" && i + 1 < args.size()) {
          fib = std::stoi(args[++i]);
        }
      }

      // Note: The routing table doesn't have a delete method yet
      // This would need to be implemented in the library
      printError("Route deletion not yet implemented in the library (FIB " +
                 std::to_string(fib) + ")");
      return false;
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleShowRouteInfo(const std::vector<std::string> &args) {
    if (args.size() < 3) {
      printError("Usage: show route <destination> [fib <number>]");
      return false;
    }

    std::string destination = args[2];
    int fib = 0;

    // Parse optional FIB parameter
    for (size_t i = 3; i < args.size(); i++) {
      if (args[i] == "fib" && i + 1 < args.size()) {
        fib = std::stoi(args[++i]);
      }
    }

    try {
      auto entries = routingTable.getEntries(fib);

      // Find matching route
      for (const auto &entry : entries) {
        if (entry->getDestination() == destination) {
          printInfo("Route: " + destination);
          printInfo("  Gateway: " + entry->getGateway());
          printInfo("  Interface: " + entry->getInterface());
          printInfo("  Flags: " + std::to_string(entry->getFlags()));
          printInfo("  FIB: " + std::to_string(fib));
          return true;
        }
      }

      printError("Route not found: " + destination + " in FIB " +
                 std::to_string(fib));
      return false;
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleFlushRoutes(const std::vector<std::string> &args) {
    int fib = 0;

    // Parse optional FIB parameter
    for (size_t i = 2; i < args.size(); i++) {
      if (args[i] == "fib" && i + 1 < args.size()) {
        fib = std::stoi(args[++i]);
      }
    }

    try {
      // Note: Route flushing would need to be implemented in the library
      printError("Route flushing not yet implemented in the library (FIB " +
                 std::to_string(fib) + ")");
      return false;
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

  bool NetTool::handleShowRouteStats(const std::vector<std::string> &args) {
    int fib = 0;

    // Parse optional FIB parameter
    for (size_t i = 2; i < args.size(); i++) {
      if (args[i] == "fib" && i + 1 < args.size()) {
        fib = std::stoi(args[++i]);
      }
    }

    try {
      auto entries = routingTable.getEntries(fib);

      printInfo("Routing Statistics for FIB " + std::to_string(fib));
      printInfo("  Total routes: " + std::to_string(entries.size()));

      // Count routes by type/interface
      std::map<std::string, int> interfaceCount;
      std::map<std::string, int> destinationCount;

      for (const auto &entry : entries) {
        interfaceCount[entry->getInterface()]++;
        destinationCount[entry->getDestination()]++;
      }

      printInfo("  Routes by interface:");
      for (const auto &pair : interfaceCount) {
        printInfo("    " + pair.first + ": " + std::to_string(pair.second));
      }

      return true;
    } catch (const std::exception &e) {
      printError("Error: " + std::string(e.what()));
      return false;
    }
  }

} // namespace net
