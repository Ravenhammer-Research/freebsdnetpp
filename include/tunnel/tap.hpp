/**
 * @file tunnel/tap.hpp
 * @brief TAP tunnel wrapper
 * @details Provides C++ wrapper for FreeBSD TAP tunnel operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_TUNNEL_TAP_HPP
#define LIBFREEBSDNET_TUNNEL_TAP_HPP

#include <memory>
#include <string>

namespace libfreebsdnet::tunnel {

  /**
   * @brief TAP tunnel class
   * @details Provides interface for managing TAP tunnels
   */
  class TapTunnel {
  public:
    TapTunnel();
    ~TapTunnel();

    // TAP-specific methods would go here

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::tunnel

#endif // LIBFREEBSDNET_TUNNEL_TAP_HPP
