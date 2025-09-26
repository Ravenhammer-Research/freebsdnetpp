/**
 * @file tunnel/gif.hpp
 * @brief GIF tunnel wrapper
 * @details Provides C++ wrapper for FreeBSD GIF tunnel operations
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_TUNNEL_GIF_HPP
#define LIBFREEBSDNET_TUNNEL_GIF_HPP

#include <memory>
#include <string>

namespace libfreebsdnet::tunnel {

  /**
   * @brief GIF tunnel class
   * @details Provides interface for managing GIF tunnels
   */
  class GifTunnel {
  public:
    GifTunnel();
    ~GifTunnel();

    // GIF-specific methods would go here

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::tunnel

#endif // LIBFREEBSDNET_TUNNEL_GIF_HPP
