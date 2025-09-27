/**
 * @file interface_commands.cpp
 * @brief Net tool interface command implementations
 * @details Implementation of interface-related command handlers for the net
 * tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/bridge.hpp>
#include <interface/lagg.hpp>
#include <interface/wireless.hpp>
#include <iostream>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net_tool.hpp>
#include <sstream>
#include <system/config.hpp>

namespace net {
  // All command implementations are now in separate files:
  // - interface_show_commands.cpp
  // - interface_set_commands.cpp
  // - interface_delete_commands.cpp
} // namespace net