/**
 * @file media/media.hpp
 * @brief Media manager wrapper
 * @details Provides C++ wrapper for FreeBSD network media management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_MEDIA_MANAGER_HPP
#define LIBFREEBSDNET_MEDIA_MANAGER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::media {

  /**
   * @brief Media type enumeration
   */
  enum class MediaType {
    UNKNOWN,
    ETHERNET,
    FDDI,
    TOKEN_RING,
    ATM,
    SERIAL,
    PPP,
    LOOPBACK,
    SLIP,
    IEEE80211,
    FIREWIRE,
    INFINIBAND
  };

  /**
   * @brief Media subtype enumeration
   */
  enum class MediaSubtype {
    UNKNOWN,
    // Ethernet subtypes
    ETHERNET_AUTO,
    ETHERNET_10BASE_T,
    ETHERNET_10BASE_T_FULL,
    ETHERNET_100BASE_TX,
    ETHERNET_100BASE_TX_FULL,
    ETHERNET_1000BASE_T,
    ETHERNET_1000BASE_T_FULL,
    ETHERNET_1000BASE_SX,
    ETHERNET_1000BASE_LX,
    ETHERNET_10GBASE_T,
    ETHERNET_10GBASE_SR,
    ETHERNET_10GBASE_LR,
    // Wireless subtypes
    IEEE80211_AUTO,
    IEEE80211_11A,
    IEEE80211_11B,
    IEEE80211_11G,
    IEEE80211_11N,
    IEEE80211_11AC,
    IEEE80211_11AX
  };

  /**
   * @brief Media option enumeration
   */
  enum class MediaOption {
    UNKNOWN,
    HALF_DUPLEX,
    FULL_DUPLEX,
    AUTO_NEG,
    FORCE_LINK,
    NO_PAUSE,
    ASYM_PAUSE,
    SYM_PAUSE,
    REM_FAULT,
    REM_FAULT_SUPPRESS,
    WOL_MAGIC,
    WOL_PHY,
    WOL_UCAST,
    WOL_MCAST,
    WOL_BCAST,
    WOL_ARP,
    WOL_TCP
  };

  /**
   * @brief Media status enumeration
   */
  enum class MediaStatus {
    UNKNOWN,
    ACTIVE,
    INACTIVE,
    AUTO_SELECTED,
    MANUAL_SELECTED,
    FORCED_UP
  };

  /**
   * @brief Media information structure
   * @details Contains media type and capability information
   */
  struct MediaInfo {
    MediaType type;
    MediaSubtype subtype;
    std::string description;
    uint64_t capability;
    std::vector<MediaOption> options;
    MediaStatus status;
    uint32_t mtu;

    MediaInfo()
        : type(MediaType::UNKNOWN), subtype(MediaSubtype::UNKNOWN),
          capability(0), status(MediaStatus::UNKNOWN), mtu(0) {}
  };

  /**
   * @brief Media manager class
   * @details Provides interface for managing network media types and options
   */
  class MediaManager {
  public:
    MediaManager();
    ~MediaManager();

    /**
     * @brief Get media information for an interface
     * @param interfaceName Interface name
     * @return Vector of media information structures
     */
    std::vector<MediaInfo> getMediaInfo(const std::string &interfaceName) const;

    /**
     * @brief Get current media selection for an interface
     * @param interfaceName Interface name
     * @return Current media information or nullptr if not found
     */
    std::unique_ptr<MediaInfo>
    getCurrentMedia(const std::string &interfaceName) const;

    /**
     * @brief Set media selection for an interface
     * @param interfaceName Interface name
     * @param mediaInfo Media information to set
     * @return true on success, false on error
     */
    bool setMedia(const std::string &interfaceName, const MediaInfo &mediaInfo);

    /**
     * @brief Enable auto-negotiation on an interface
     * @param interfaceName Interface name
     * @return true on success, false on error
     */
    bool enableAutoNegotiation(const std::string &interfaceName);

    /**
     * @brief Disable auto-negotiation on an interface
     * @param interfaceName Interface name
     * @return true on success, false on error
     */
    bool disableAutoNegotiation(const std::string &interfaceName);

    /**
     * @brief Check if auto-negotiation is enabled
     * @param interfaceName Interface name
     * @return true if auto-negotiation is enabled, false otherwise
     */
    bool isAutoNegotiationEnabled(const std::string &interfaceName) const;

    /**
     * @brief Set media options for an interface
     * @param interfaceName Interface name
     * @param options Vector of media options to set
     * @return true on success, false on error
     */
    bool setMediaOptions(const std::string &interfaceName,
                         const std::vector<MediaOption> &options);

    /**
     * @brief Get media options for an interface
     * @param interfaceName Interface name
     * @return Vector of current media options
     */
    std::vector<MediaOption>
    getMediaOptions(const std::string &interfaceName) const;

    /**
     * @brief Check if a media option is supported
     * @param interfaceName Interface name
     * @param option Media option to check
     * @return true if option is supported, false otherwise
     */
    bool isMediaOptionSupported(const std::string &interfaceName,
                                MediaOption option) const;

    /**
     * @brief Get media type string
     * @param type Media type
     * @return String representation of media type
     */
    static std::string getMediaTypeString(MediaType type);

    /**
     * @brief Get media subtype string
     * @param subtype Media subtype
     * @return String representation of media subtype
     */
    static std::string getMediaSubtypeString(MediaSubtype subtype);

    /**
     * @brief Get media option string
     * @param option Media option
     * @return String representation of media option
     */
    static std::string getMediaOptionString(MediaOption option);

    /**
     * @brief Get media status string
     * @param status Media status
     * @return String representation of media status
     */
    static std::string getMediaStatusString(MediaStatus status);

    /**
     * @brief Parse media type from string
     * @param typeString Media type string
     * @return Media type or UNKNOWN if invalid
     */
    static MediaType parseMediaType(const std::string &typeString);

    /**
     * @brief Parse media subtype from string
     * @param subtypeString Media subtype string
     * @return Media subtype or UNKNOWN if invalid
     */
    static MediaSubtype parseMediaSubtype(const std::string &subtypeString);

    /**
     * @brief Get last error message
     * @return Error message from last operation
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::media

#endif // LIBFREEBSDNET_MEDIA_MANAGER_HPP
