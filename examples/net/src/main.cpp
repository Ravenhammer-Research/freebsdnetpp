/**
 * @file main.cpp
 * @brief Net tool main function
 * @details Main entry point for the net command-line tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <cstdlib>
#include <iostream>
#include <net_tool.hpp>

int main(int argc, char *argv[]) {
  try {
    net::NetTool tool;
    return tool.run(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown fatal error occurred" << std::endl;
    return 1;
  }
}
