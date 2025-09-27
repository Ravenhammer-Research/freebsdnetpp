/**
 * @file utils.cpp
 * @brief Net tool utility functions
 * @details Implementation of utility functions for the net tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <net_tool.hpp>
#include <sstream>

namespace net {

  std::vector<std::string> NetTool::splitCommand(const std::string &command) {
    std::vector<std::string> args;
    std::istringstream iss(command);
    std::string arg;

    while (iss >> arg) {
      args.push_back(arg);
    }

    return args;
  }

  void NetTool::printError(const std::string &message) {
    std::cout << "\033[31mError: " << message << "\033[0m" << std::endl;
  }

  void NetTool::printSuccess(const std::string &message) {
    std::cout << "\033[32m" << message << "\033[0m" << std::endl;
  }

  void NetTool::printInfo(const std::string &message) {
    std::cout << "\033[36m" << message << "\033[0m" << std::endl;
  }

  void NetTool::printTable(const std::vector<std::vector<std::string>> &data,
                           const std::vector<std::string> &headers) {
    if (data.empty()) {
      return;
    }

    // Calculate column widths
    std::vector<size_t> widths(headers.size(), 0);
    for (size_t i = 0; i < headers.size(); i++) {
      widths[i] = headers[i].length();
    }

    for (const auto &row : data) {
      for (size_t i = 0; i < row.size() && i < widths.size(); i++) {
        widths[i] = std::max(widths[i], row[i].length());
      }
    }

    // Helper function to center text in a field
    auto centerText = [](const std::string &text, size_t width) {
      if (text.length() >= width) {
        return text;
      }
      size_t padding = width - text.length();
      size_t leftPadding = padding / 2;
      size_t rightPadding = padding - leftPadding;
      return std::string(leftPadding, ' ') + text +
             std::string(rightPadding, ' ');
    };

    // Print header
    for (size_t i = 0; i < headers.size(); i++) {
      std::cout << centerText(headers[i], widths[i] + 1);
    }
    std::cout << std::endl;

    // Print separator
    for (size_t i = 0; i < headers.size(); i++) {
      std::cout << std::string(widths[i] + 1, '-');
    }
    std::cout << std::endl;

    // Print data
    for (const auto &row : data) {
      for (size_t i = 0; i < row.size() && i < widths.size(); i++) {
        std::cout << centerText(row[i], widths[i] + 1);
      }
      std::cout << std::endl;
    }
  }

} // namespace net
