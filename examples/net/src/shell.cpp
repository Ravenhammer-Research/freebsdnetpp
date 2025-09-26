/**
 * @file shell.cpp
 * @brief Net tool interactive shell implementation
 * @details Implementation of the interactive shell for the net tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <iostream>
#include <net_tool.hpp>
#include <readline/history.h>
#include <readline/readline.h>

namespace net {

  int NetTool::runInteractive() {
    std::cout << "Net Tool - FreeBSD Network Management" << std::endl;
    std::cout << "Type 'help' for available commands, 'exit' or 'quit' to exit."
              << std::endl;
    std::cout << std::endl;

    char *line;
    while ((line = readline(prompt.c_str())) != nullptr) {
      std::string command(line);
      free(line);

      if (command.empty()) {
        continue;
      }

      // Add to history
      add_history(command.c_str());

      // Handle special commands
      if (command == "exit" || command == "quit") {
        break;
      }

      if (command == "clear") {
        std::cout << "\033[2J\033[1;1H"; // Clear screen
        continue;
      }

      // Execute command
      if (!executeCommand(command)) {
        // Error already printed by command handler
      }
    }

    return 0;
  }

  bool NetTool::executeCommand(const std::string &command) {
    auto args = splitCommand(command);
    if (args.empty()) {
      return true;
    }

    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    auto it = commands.find(cmd);
    if (it == commands.end()) {
      printError("Unknown command: " + args[0]);
      printInfo("Type 'help' for available commands.");
      return false;
    }

    return it->second.handler(args);
  }

} // namespace net
