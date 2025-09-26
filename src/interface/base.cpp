/**
 * @file interface/base.cpp
 * @brief Base interface class implementation
 * @details Implementation of base interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/base.hpp>
#include <interface/factory.hpp>
#include <memory>
#include <net/if.h>
#include <string>

namespace libfreebsdnet::interface {

  std::unique_ptr<Interface> createInterface(const std::string &name,
                                             unsigned int index, int flags) {
    return InterfaceFactory::createInterface(name, index, flags);
  }

  InterfaceType getInterfaceType(const std::string &name, int flags) {
    return InterfaceFactory::getInterfaceType(name, flags);
  }

} // namespace libfreebsdnet::interface
