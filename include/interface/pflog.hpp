/**
 * @file interface/pflog.hpp
 * @brief PFLOG interface class
 * @details C++ wrapper for PFLOG network interfaces
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_INTERFACE_PFLOG_HPP
#define LIBFREEBSDNET_INTERFACE_PFLOG_HPP

#include "base.hpp"
#include <string>

namespace libfreebsdnet::interface {

  /**
   * @brief PFLOG interface class
   * @details Provides PFLOG-specific interface operations
   */
  class PflogInterface : public Interface {
  public:
    /**
     * @brief Constructor
     * @param name Interface name (e.g., "pflog0")
     * @param index Interface index
     * @param flags Interface flags
     */
    PflogInterface(const std::string &name, unsigned int index, int flags);

    /**
     * @brief Destructor
     */
    ~PflogInterface() override;

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

    /**
     * @brief Get log interface
     * @return Log interface name or empty string on error
     */
    std::string getLogInterface() const;

    /**
     * @brief Set log interface
     * @param interfaceName Log interface name
     * @return true on success, false on error
     */
    bool setLogInterface(const std::string &interfaceName);

    /**
     * @brief Get log rule number
     * @return Log rule number or -1 on error
     */
    int getLogRule() const;

    /**
     * @brief Set log rule number
     * @param ruleNumber Log rule number
     * @return true on success, false on error
     */
    bool setLogRule(int ruleNumber);

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

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::interface

#endif // LIBFREEBSDNET_INTERFACE_PFLOG_HPP
