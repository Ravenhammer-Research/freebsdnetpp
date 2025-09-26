/**
 * @file netlink/manager.hpp
 * @brief Netlink manager header
 * @details Header for netlink interface management functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef LIBFREEBSDNET_NETLINK_MANAGER_HPP
#define LIBFREEBSDNET_NETLINK_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace libfreebsdnet::netlink {

  /**
   * @brief Netlink message types
   */
  enum class NetlinkMessageType {
    NEWLINK = 16,  // RTM_NEWLINK
    DELLINK = 17,  // RTM_DELLINK
    GETLINK = 18,  // RTM_GETLINK
    SETLINK = 19,  // RTM_SETLINK
    NEWADDR = 20,  // RTM_NEWADDR
    DELADDR = 21,  // RTM_DELADDR
    GETADDR = 22,  // RTM_GETADDR
    NEWROUTE = 24, // RTM_NEWROUTE
    DELROUTE = 25, // RTM_DELROUTE
    GETROUTE = 26  // RTM_GETROUTE
  };

  /**
   * @brief Netlink interface information
   */
  struct NetlinkInterfaceInfo {
    std::string name;
    int index;
    int type;
    uint32_t flags;
    uint32_t change;
    std::string hardwareAddress;
    int mtu;
    std::string operstate;
  };

  /**
   * @brief Netlink message callback function type
   */
  using NetlinkCallback = std::function<void(const NetlinkInterfaceInfo &)>;

  /**
   * @brief Netlink manager class
   * @details Provides netlink interface management functionality
   */
  class NetlinkManager {
  public:
    /**
     * @brief Constructor
     */
    NetlinkManager();

    /**
     * @brief Destructor
     */
    ~NetlinkManager();

    /**
     * @brief Check if netlink is available
     * @return true if available, false otherwise
     */
    bool isAvailable() const;

    /**
     * @brief Get all interfaces via netlink
     * @return Vector of interface information
     */
    std::vector<NetlinkInterfaceInfo> getInterfaces() const;

    /**
     * @brief Get interface by name via netlink
     * @param name Interface name
     * @return Interface information or empty struct if not found
     */
    NetlinkInterfaceInfo getInterface(const std::string &name) const;

    /**
     * @brief Get interface by index via netlink
     * @param index Interface index
     * @return Interface information or empty struct if not found
     */
    NetlinkInterfaceInfo getInterface(int index) const;

    /**
     * @brief Set interface flags via netlink
     * @param name Interface name
     * @param flags New flags
     * @return true on success, false on error
     */
    bool setInterfaceFlags(const std::string &name, uint32_t flags);

    /**
     * @brief Set interface MTU via netlink
     * @param name Interface name
     * @param mtu New MTU
     * @return true on success, false on error
     */
    bool setInterfaceMtu(const std::string &name, int mtu);

    /**
     * @brief Start monitoring interface changes
     * @param callback Callback function for interface changes
     * @return true on success, false on error
     */
    bool startMonitoring(const NetlinkCallback &callback);

    /**
     * @brief Stop monitoring interface changes
     * @return true on success, false on error
     */
    bool stopMonitoring();

    /**
     * @brief Get last error message
     * @return Last error message
     */
    std::string getLastError() const;

  private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
  };

} // namespace libfreebsdnet::netlink

#endif // LIBFREEBSDNET_NETLINK_MANAGER_HPP
