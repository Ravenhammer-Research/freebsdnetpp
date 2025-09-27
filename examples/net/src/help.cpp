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
    std::cout << std::endl;
    
    std::cout << "SHOW COMMANDS:" << std::endl;
    std::cout << "  show interface                     Show all network interfaces" << std::endl;
    std::cout << "  show interface <name>              Show interface details" << std::endl;
    std::cout << "  show interface <name> <property>   Show specific property" << std::endl;
    std::cout << "  show interface type <type>         Show interfaces by type" << std::endl;
    std::cout << "  show route [fib <number>]          Show routing table" << std::endl;
    std::cout << "  show route <dest> [fib <num>]      Show specific route details" << std::endl;
    std::cout << "  show route stats [fib <num>]       Show routing statistics" << std::endl;
    std::cout << "  show system                         Show system network configuration" << std::endl;
    std::cout << std::endl;
    
    std::cout << "SET COMMANDS:" << std::endl;
    std::cout << "  set interface <name> state <up|down>     Bring interface up/down" << std::endl;
    std::cout << "  set interface <name> mtu <size>          Set interface MTU" << std::endl;
    std::cout << "  set interface <name> fib <num>           Set interface FIB" << std::endl;
    std::cout << "  set interface <name> address <ip>        Set interface IP address" << std::endl;
    std::cout << "  set interface <name> media <val>         Set interface media" << std::endl;
    std::cout << "  set interface <name> capabilities <val>  Set interface capabilities" << std::endl;
    std::cout << "  set interface <name> vnet <id>           Set interface VNET" << std::endl;
    std::cout << "  set interface <name> mac <addr>          Set interface MAC address" << std::endl;
    std::cout << "  set interface <name> <ipv6_option> <enable|disable>  Set IPv6 options" << std::endl;
    std::cout << "  set route <dest> <gw> [interface] [fib <num>]  Add route" << std::endl;
    std::cout << "  set bridge <name> stp <enable|disable>   Configure bridge STP" << std::endl;
    std::cout << "  set bridge <name> addm <interface>       Add bridge member" << std::endl;
    std::cout << "  set bridge <name> delm <interface>       Remove bridge member" << std::endl;
    std::cout << "  set lagg <name> protocol <proto>         Set LAGG protocol" << std::endl;
    std::cout << "  set lagg <name> addm <interface>         Add LAGG member" << std::endl;
    std::cout << "  set lagg <name> delm <interface>         Remove LAGG member" << std::endl;
    std::cout << std::endl;
    
    std::cout << "ADD COMMANDS:" << std::endl;
    std::cout << "  add route <dest> <gw> [interface] [fib <num>]  Add route" << std::endl;
    std::cout << std::endl;
    
    std::cout << "DELETE COMMANDS:" << std::endl;
    std::cout << "  delete interface <name> <property> [value]     Remove interface property" << std::endl;
    std::cout << "  delete bridge <name> <property> [value]        Remove bridge property" << std::endl;
    std::cout << "  delete lagg <name> <property> [value]          Remove LAGG property" << std::endl;
    std::cout << "  delete system <property>                       Reset system property to default" << std::endl;
    std::cout << "  delete route <dest> [fib <num>]                Delete route" << std::endl;
    std::cout << std::endl;
    
    std::cout << "FLUSH COMMANDS:" << std::endl;
    std::cout << "  flush route [fib <num>]                        Flush routing table" << std::endl;
    std::cout << std::endl;
    
    std::cout << "SAVE COMMANDS:" << std::endl;
    std::cout << "  save state                                     Save current network state" << std::endl;
    std::cout << std::endl;
    
    std::cout << "UTILITY COMMANDS:" << std::endl;
    std::cout << "  help [command]                                 Show help for command" << std::endl;
    std::cout << "  exit/quit                                      Exit the program" << std::endl;
    std::cout << "  clear                                          Clear the screen" << std::endl;
    std::cout << std::endl;
    
    std::cout << "IPv6 OPTIONS:" << std::endl;
    std::cout << "  accept_rtadv, auto_linklocal, perform_nud, slaac, ifdisabled, no_radr, no_dad" << std::endl;
    std::cout << std::endl;
    
    std::cout << "LAGG PROTOCOLS:" << std::endl;
    std::cout << "  lacp, failover, loadbalance, roundrobin, fec" << std::endl;
    std::cout << std::endl;
    
    std::cout << "INTERFACE TYPES:" << std::endl;
    std::cout << "  bridge, lagg, gif, ethernet" << std::endl;
    std::cout << std::endl;
    
    std::cout << "EXAMPLES:" << std::endl;
    std::cout << "  show interface lagg0                          Show LAGG interface details" << std::endl;
    std::cout << "  set interface re0 state up                    Bring interface up" << std::endl;
    std::cout << "  set interface re0 address 192.168.1.100/24    Set IP address" << std::endl;
    std::cout << "  set interface re0 slaac enable                Enable IPv6 SLAAC" << std::endl;
    std::cout << "  set lagg lagg0 protocol lacp                  Set LAGG protocol" << std::endl;
    std::cout << "  set route 0.0.0.0 192.168.1.1 re0            Add default route" << std::endl;
    std::cout << "  save state > config.txt                       Save configuration" << std::endl;
    std::cout << "  net -c - < config.txt                         Restore configuration" << std::endl;
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
