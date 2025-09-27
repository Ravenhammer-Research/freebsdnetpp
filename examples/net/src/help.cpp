/**
 * @file help.cpp
 * @brief Net tool help and utility functions
 * @details Implementation of help system and utility functions for the net tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <algorithm>
#include <iostream>
#include <net_tool.hpp>

namespace net {

  void NetTool::showHelp() {
    std::cout << "Net Tool - FreeBSD Network Management" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: net [options] [command]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help          Show this help message" << std::endl;
    std::cout << "  -v, --version       Show version information" << std::endl;
    std::cout << "  -i, --interactive   Start interactive shell" << std::endl;
    std::cout << "  -c, --command CMD   Execute single command" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout
        << "  show interfaces                    Show all network interfaces"
        << std::endl;
    std::cout << "  show interface <name> [property]  Show interface details"
              << std::endl;
    std::cout << "  show route [fib <number>]          Show routing table"
              << std::endl;
    std::cout
        << "  show route <dest> [fib <num>]      Show specific route details"
        << std::endl;
    std::cout << "  show route stats [fib <num>]       Show routing statistics"
              << std::endl;
    std::cout << "  show system                         Show system network "
                 "configuration"
              << std::endl;
    std::cout << "  set interface <name> fib <num>     Set interface FIB"
              << std::endl;
    std::cout
        << "  set interface <name> member <if>   Add interface to bridge/lagg"
        << std::endl;
    std::cout << "  set interface <name> mode <mode>   Set interface mode "
                 "(lacp, etc.)"
              << std::endl;
    std::cout << "  set interface <name> address <ip>  Set interface IP address"
              << std::endl;
    std::cout << "  set interface <name> mtu <size>    Set interface MTU"
              << std::endl;
    std::cout << "  set interface <name> up|down       Bring interface up/down"
              << std::endl;
    std::cout << "  set interface <name> media <val>   Set interface media"
              << std::endl;
    std::cout << "  set interface <name> capabilities <val> Set interface "
                 "capabilities"
              << std::endl;
    std::cout << "  set interface <name> vnet <id>     Set interface VNET"
              << std::endl;
    std::cout
        << "  set interface <name> mac <addr>    Set interface MAC address"
        << std::endl;
    std::cout << "  set route <dest> <gw> [fib <num>]  Set/add route"
              << std::endl;
    std::cout << "  add route <dest> <gw> [fib <num>] Add route" << std::endl;
    std::cout << "  delete interface <name> fib        Remove interface FIB "
                 "configuration"
              << std::endl;
    std::cout << "  delete interface <name>            Remove interface"
              << std::endl;
    std::cout << "  delete route <dest> [fib <num>]    Delete route"
              << std::endl;
    std::cout << "  flush route [fib <num>]            Flush routing table"
              << std::endl;
    std::cout << "  help [command]                     Show help for command"
              << std::endl;
    std::cout << "  exit/quit                          Exit the program"
              << std::endl;
    std::cout << "  clear                              Clear the screen"
              << std::endl;
  }

  void NetTool::showVersion() {
    std::cout << "Net Tool v1.0.0" << std::endl;
    std::cout << "Built with libfreebsdnet++" << std::endl;
  }

  bool NetTool::handleHelp(const std::vector<std::string> &args) {
    if (args.size() > 1) {
      std::string cmd = args[1];
      std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

      auto it = commands.find(cmd);
      if (it != commands.end()) {
        std::cout << it->second.name << " - " << it->second.description
                  << std::endl;
        std::cout << "Usage: " << it->second.usage << std::endl;
      } else {
        printError("Unknown command: " + cmd);
        return false;
      }
    } else {
      showHelp();
    }

    return true;
  }

  bool NetTool::handleExit(const std::vector<std::string> &args) {
    (void)args;  // Suppress unused parameter warning
    return true; // Signal to exit
  }

  bool NetTool::handleQuit(const std::vector<std::string> &args) {
    (void)args;  // Suppress unused parameter warning
    return true; // Signal to exit
  }

  bool NetTool::handleClear(const std::vector<std::string> &args) {
    (void)args;                      // Suppress unused parameter warning
    std::cout << "\033[2J\033[1;1H"; // Clear screen
    return true;
  }

} // namespace net
