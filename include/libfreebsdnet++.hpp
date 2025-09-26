/**
 * @file libfreebsdnet++.hpp
 * @brief Main library header for libfreebsdnet++
 * @details C++ wrapper library for FreeBSD network functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_HPP
#define LIBFREEBSDNET_HPP

// Include all library components
#include <altq/lib.hpp>
#include <bpf/lib.hpp>
#include <bridge/lib.hpp>
#include <ethernet/lib.hpp>
#include <interface/lib.hpp>
#include <media/lib.hpp>
#include <netmap/lib.hpp>
#include <routing/lib.hpp>
#include <tunnel/lib.hpp>
#include <vlan/lib.hpp>

/**
 * @brief libfreebsdnet++ namespace
 * @details Main namespace for the libfreebsdnet++ library
 */
namespace libfreebsdnet {

  /**
   * @brief Library version information
   */
  constexpr const char *VERSION = "1.0.0";

  /**
   * @brief Library build information
   */
  constexpr const char *BUILD_DATE = __DATE__ " " __TIME__;

  /**
   * @brief Get library version
   * @return Library version string
   */
  inline const char *getVersion() { return VERSION; }

  /**
   * @brief Get library build date
   * @return Library build date string
   */
  inline const char *getBuildDate() { return BUILD_DATE; }

} // namespace libfreebsdnet

#endif // LIBFREEBSDNET_HPP
