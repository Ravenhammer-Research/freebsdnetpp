/**
 * @file interface/vnet.cpp
 * @brief VNET interface mix-in implementation
 * @details Implementation of VNET functionality for interfaces that support it
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <cstring>
#include <interface/vnet.hpp>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  // Note: This is a mix-in class, so implementations will be provided
  // by the concrete interface classes that inherit from it.
  // The actual VNET functionality is implemented in the specific interface
  // classes that support VNET operations.

} // namespace libfreebsdnet::interface
