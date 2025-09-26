/**
 * @file interface/carp.hpp
 * @brief CARP interface header
 * @details Header for CARP (Common Address Redundancy Protocol) interface
 * functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#pragma once

#include <interface/base.hpp>
#include <memory>
#include <netinet/ip_carp.h>
#include <string>
#include <vector>

namespace libfreebsdnet::interface {

  /**
   * @brief CARP state enumeration
   */
  enum class CarpState { INIT, BACKUP, MASTER };

  /**
   * @brief CARP interface class
   * @details Implementation of CARP interface functionality
   */
  class CarpInterface : public Interface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name
     * @param index Interface index
     * @param flags Interface flags
     */
    CarpInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~CarpInterface() override;

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

    int getMedia() const override;
    bool setMedia(int media) override;
    int getMediaStatus() const override;
    int getActiveMedia() const override;
    std::vector<int> getSupportedMedia() const override;
    uint32_t getCapabilities() const override;
    bool setCapabilities(uint32_t capabilities) override;
    uint32_t getEnabledCapabilities() const override;
    bool enableCapabilities(uint32_t capabilities) override;
    bool disableCapabilities(uint32_t capabilities) override;
    std::vector<std::string> getGroups() const override;
    bool addToGroup(const std::string &groupName) override;
    bool removeFromGroup(const std::string &groupName) override;
    bool setPhysicalAddress(const std::string &address) override;
    bool deletePhysicalAddress() override;
    bool createClone(const std::string &cloneName) override;
    std::vector<std::string> getCloners() const override;
    std::string getMacAddress() const override;
    bool setMacAddress(const std::string &macAddress) override;

    bool destroy() override;

    // CARP-specific methods
    /**
     * @brief Get CARP VHID (Virtual Host ID)
     * @return VHID or -1 if not set
     */
    int getVhid() const;

    /**
     * @brief Set CARP VHID
     * @param vhid Virtual Host ID (1-255)
     * @return true on success, false on error
     */
    bool setVhid(int vhid);

    /**
     * @brief Get CARP state
     * @return Current CARP state
     */
    CarpState getState() const;

    /**
     * @brief Get CARP advertisement base
     * @return Advertisement base in seconds
     */
    int getAdvBase() const;

    /**
     * @brief Set CARP advertisement base
     * @param advbase Advertisement base in seconds
     * @return true on success, false on error
     */
    bool setAdvBase(int advbase);

    /**
     * @brief Get CARP advertisement skew
     * @return Advertisement skew
     */
    int getAdvSkew() const;

    /**
     * @brief Set CARP advertisement skew
     * @param advskew Advertisement skew
     * @return true on success, false on error
     */
    bool setAdvSkew(int advskew);

    /**
     * @brief Get CARP peer address
     * @return Peer address as string
     */
    std::string getPeerAddress() const;

    /**
     * @brief Set CARP peer address
     * @param peer Peer address
     * @return true on success, false on error
     */
    bool setPeerAddress(const std::string &peer);

    /**
     * @brief Get CARP peer IPv6 address
     * @return Peer IPv6 address as string
     */
    std::string getPeerAddress6() const;

    /**
     * @brief Set CARP peer IPv6 address
     * @param peer6 Peer IPv6 address
     * @return true on success, false on error
     */
    bool setPeerAddress6(const std::string &peer6);

    /**
     * @brief Get CARP authentication key
     * @return Authentication key
     */
    std::string getKey() const;

    /**
     * @brief Set CARP authentication key
     * @param key Authentication key
     * @return true on success, false on error
     */
    bool setKey(const std::string &key);

    /**
     * @brief Check if CARP interface is valid
     * @return true if valid, false otherwise
     */
    bool isValid() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface
