/**
 * @file tunnel/gre.hpp
 * @brief GRE tunnel wrapper
 * @details Provides C++ wrapper for FreeBSD GRE tunnel operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_TUNNEL_GRE_HPP
#define LIBFREEBSDNET_TUNNEL_GRE_HPP

#include <memory>
#include <string>

namespace libfreebsdnet::tunnel {

  /**
   * @brief GRE tunnel class
   * @details Provides interface for managing GRE tunnels
   */
  class GreTunnel {
  public:
    GreTunnel();
    ~GreTunnel();

    // GRE-specific methods would go here

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::tunnel

#endif // LIBFREEBSDNET_TUNNEL_GRE_HPP
