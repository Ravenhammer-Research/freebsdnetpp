/**
 * @file net_tool.cpp
 * @brief Net command-line tool main implementation
 * @details Main implementation of the net command-line tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <iostream>
#include <net_tool.hpp>

namespace net {

  NetTool::NetTool() { initializeCommands(); }

  NetTool::~NetTool() = default;

  int NetTool::run(int argc, char *argv[]) {
    if (!parseArguments(argc, argv)) {
      return 1;
    }

    if (interactive) {
      return runInteractive();
    }

    // Execute single command
    std::string command;
    for (int i = 1; i < argc; i++) {
      if (i > 1)
        command += " ";
      command += argv[i];
    }

    if (command.empty()) {
      showHelp();
      return 0;
    }

    return executeCommand(command) ? 0 : 1;
  }

} // namespace net
