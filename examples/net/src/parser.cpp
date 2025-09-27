/**
 * @file parser.cpp
 * @brief Net tool command parser implementation
 * @details Implementation of command parsing and initialization for the net
 * tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <algorithm>
#include <iostream>
#include <net_tool.hpp>

namespace net {

  bool NetTool::parseArguments(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
      std::string arg = argv[i];

      if (arg == "-h" || arg == "--help") {
        showHelp();
        return false;
      } else if (arg == "-v" || arg == "--version") {
        showVersion();
        return false;
      } else if (arg == "-i" || arg == "--interactive") {
        interactive = true;
      } else if (arg == "-c" || arg == "--command") {
        if (i + 1 < argc) {
          std::string command = argv[++i];
          return executeCommand(command);
        } else {
          printError("Missing command after -c/--command");
          return false;
        }
      }
    }

    return true;
  }

  void NetTool::initializeCommands() {
    commands["show"] = {
        "show", "Show information (interfaces, routes, etc.)",
        [this](const std::vector<std::string> &args) {
          if (args.size() < 2) {
            printError("Usage: show <interface|route> [options]");
            return false;
          }

          if (args[1] == "interfaces" ||
              (args[1] == "interface" && args.size() == 2)) {
            return handleShowInterfaces(args);
          } else if (args[1] == "interface" && args.size() > 2) {
            if (args[2] == "type" && args.size() > 3) {
              return handleShowInterfaceType(args);
            } else {
              return handleShowInterfaceInfo(args);
            }
          } else if (args[1] == "route") {
            if (args.size() > 2 && args[2] == "stats") {
              return handleShowRouteStats(args);
            } else {
              return handleShowRoute(args);
            }
          } else if (args[1] == "system") {
            return handleShowSystem(args);
          } else {
            printError("Unknown show target: " + args[1]);
            return false;
          }
        },
        "show <interface|route> [options]"};

    commands["set"] = {
        "set", "Set interface or route properties",
        [this](const std::vector<std::string> &args) {
          if (args.size() < 2) {
            printError("Usage: set <interface|interfaces|route> <name> "
                       "<property> <value>");
            return false;
          }

          if (args[1] == "interface" || args[1] == "interfaces") {
            return handleSetInterface(args);
          } else if (args[1] == "route") {
            return handleSetRoute(args);
          } else if (args[1] == "system") {
            return handleSetSystem(args);
          } else {
            printError("Unknown set target: " + args[1]);
            return false;
          }
        },
        "set <interface|interfaces|route> <name> <property> <value>"};

    commands["delete"] = {
        "delete", "Delete interface, interface properties, or routes",
        [this](const std::vector<std::string> &args) {
          if (args.size() < 2) {
            printError(
                "Usage: delete <interface|interfaces|route> <name> [property]");
            return false;
          }

          if (args[1] == "interface" || args[1] == "interfaces") {
            return handleDeleteInterface(args);
          } else if (args[1] == "bridge") {
            return handleDeleteBridge(args);
          } else if (args[1] == "lagg") {
            return handleDeleteLagg(args);
          } else if (args[1] == "system") {
            return handleDeleteSystem(args);
          } else if (args[1] == "route") {
            return handleDeleteRoute(args);
          } else {
            printError("Unknown delete target: " + args[1]);
            return false;
          }
        },
        "delete <interface|interfaces|route> <name> [property]"};

    commands["del"] = {
        "del",
        "Delete interface, interface properties, or routes (alias for delete)",
        [this](const std::vector<std::string> &args) {
          return commands["delete"].handler(args);
        },
        "del <interface|interfaces|route> <name> [property]"};

    commands["add"] = {
        "add", "Add routes",
        [this](const std::vector<std::string> &args) {
          if (args.size() < 2) {
            printError("Usage: add <route> <destination> <gateway> [interface] "
                       "[fib <number>]");
            return false;
          }

          if (args[1] == "route") {
            return handleAddRoute(args);
          } else {
            printError("Unknown add target: " + args[1]);
            return false;
          }
        },
        "add route <destination> <gateway> [interface] [fib <number>]"};

    commands["help"] = {"help", "Show help information",
                        [this](const std::vector<std::string> &args) {
                          return handleHelp(args);
                        },
                        "help [command]"};

    commands["exit"] = {"exit", "Exit the program",
                        [this](const std::vector<std::string> &args) {
                          return handleExit(args);
                        },
                        "exit"};

    commands["quit"] = {"quit", "Exit the program",
                        [this](const std::vector<std::string> &args) {
                          return handleQuit(args);
                        },
                        "quit"};

    commands["clear"] = {"clear", "Clear the screen",
                         [this](const std::vector<std::string> &args) {
                           return handleClear(args);
                         },
                         "clear"};

    commands["flush"] = {"flush", "Flush routes",
                         [this](const std::vector<std::string> &args) {
                           if (args.size() < 2) {
                             printError("Usage: flush <route> [fib <number>]");
                             return false;
                           }

                           if (args[1] == "route") {
                             return handleFlushRoutes(args);
                           } else {
                             printError("Unknown flush target: " + args[1]);
                             return false;
                           }
                         },
                         "flush route [fib <number>]"};

    commands["save"] = {"save", "Save current network state",
                        [this](const std::vector<std::string> &args) {
                          if (args.size() < 2) {
                            printError("Usage: save <state>");
                            return false;
                          }

                          if (args[1] == "state") {
                            return handleSaveState(args);
                          } else {
                            printError("Unknown save target: " + args[1]);
                            return false;
                          }
                        },
                        "save <state>"};
  }

} // namespace net
