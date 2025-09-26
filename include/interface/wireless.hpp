/**
 * @file interface/wireless.hpp
 * @brief IEEE 802.11 wireless interface wrapper
 * @details Provides C++ wrapper for FreeBSD IEEE 802.11 wireless interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_WIRELESS_HPP
#define LIBFREEBSDNET_INTERFACE_WIRELESS_HPP

#include <cstdint>
#include <interface/base.hpp>
#include <interface/vnet.hpp>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief IEEE 802.11 wireless interface class
   * @details Provides management for IEEE 802.11 wireless network interfaces
   */
  class WirelessInterface : public Interface, public VnetInterface {
  public:
    WirelessInterface(const std::string &name, unsigned int index, int flags);
    ~WirelessInterface() override;

    // Base class method overrides
    std::string getName() const override;
    unsigned int getIndex() const override;
    InterfaceType getType() const override;
    std::vector<Flag> getFlags() const override;
    bool setFlags(int flags) override;
    bool bringUp() override;
    bool bringDown() override;
    bool isUp() const override;
    int getMtu() const override;
    bool setMtu(int mtu) override;
    std::string getLastError() const override;
    int getFib() const override;
    bool setFib(int fib) override;

    bool isValid() const;

    // Media support
    int getMedia() const override;
    bool setMedia(int media) override;
    int getMediaStatus() const override;
    int getActiveMedia() const override;
    std::vector<int> getSupportedMedia() const override;

    // Capability support
    uint32_t getCapabilities() const override;
    bool setCapabilities(uint32_t capabilities) override;
    uint32_t getEnabledCapabilities() const override;
    bool enableCapabilities(uint32_t capabilities) override;
    bool disableCapabilities(uint32_t capabilities) override;

    // Group support
    std::vector<std::string> getGroups() const override;
    bool addToGroup(const std::string &groupName) override;
    bool removeFromGroup(const std::string &groupName) override;

    // VNET support
    int getVnet() const override;
    std::string getVnetJailName() const override;
    bool setVnet(int vnetId) override;
    bool reclaimFromVnet() override;

    // Physical address support
    bool setPhysicalAddress(const std::string &address) override;
    bool deletePhysicalAddress() override;

    // Interface cloning support
    bool createClone(const std::string &cloneName) override;
    std::vector<std::string> getCloners() const override;

    // MAC address support
    std::string getMacAddress() const override;
    bool setMacAddress(const std::string &macAddress) override;


    bool destroy() override;

    // IEEE 802.11-specific methods
    /**
     * @brief Get current channel
     * @return Current channel number or -1 on error
     */
    int getChannel() const;

    /**
     * @brief Set channel
     * @param channel Channel number
     * @return true on success, false on error
     */
    bool setChannel(int channel);

    /**
     * @brief Get SSID
     * @return SSID string
     */
    std::string getSsid() const;

    /**
     * @brief Set SSID
     * @param ssid SSID string
     * @return true on success, false on error
     */
    bool setSsid(const std::string &ssid);

    /**
     * @brief Get wireless mode
     * @return Wireless mode string
     */
    std::string getMode() const;

    /**
     * @brief Set wireless mode
     * @param mode Wireless mode (sta, ap, adhoc, etc.)
     * @return true on success, false on error
     */
    bool setMode(const std::string &mode);

    /**
     * @brief Get signal strength
     * @return Signal strength in dBm or -1 on error
     */
    int getSignalStrength() const;

    /**
     * @brief Get noise level
     * @return Noise level in dBm or -1 on error
     */
    int getNoiseLevel() const;

    /**
     * @brief Get supported rates
     * @return Vector of supported rates in Mbps
     */
    std::vector<int> getSupportedRates() const;

    /**
     * @brief Get current rate
     * @return Current rate in Mbps or -1 on error
     */
    int getCurrentRate() const;

    /**
     * @brief Get encryption status
     * @return true if encryption is enabled, false otherwise
     */
    bool isEncryptionEnabled() const;

    /**
     * @brief Get available networks
     * @return Vector of available network information
     */
    std::vector<std::string> getAvailableNetworks() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_WIRELESS_HPP
