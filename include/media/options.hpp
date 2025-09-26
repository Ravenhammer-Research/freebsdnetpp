/**
 * @file media/options.hpp
 * @brief Media options wrapper
 * @details Provides C++ wrapper for FreeBSD media options
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_MEDIA_OPTIONS_HPP
#define LIBFREEBSDNET_MEDIA_OPTIONS_HPP

#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::media {

  /**
   * @brief Media options class
   * @details Provides interface for managing media options
   */
  class MediaOptions {
  public:
    MediaOptions();
    ~MediaOptions();

    // Media options methods would go here

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::media

#endif // LIBFREEBSDNET_MEDIA_OPTIONS_HPP
