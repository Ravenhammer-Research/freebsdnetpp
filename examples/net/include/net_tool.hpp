/**
 * @file net_tool.hpp
 * @brief Net command-line tool header
 * @details Command-line tool for network interface and routing management
 *
 * @author paigeadelethompson
 * @year 2024
 */

#ifndef NET_TOOL_HPP
#define NET_TOOL_HPP

#include <functional>
#include <interface/manager.hpp>
#include <map>
#include <memory>
#include <netlink/manager.hpp>
#include <routing/table.hpp>
#include <string>
#include <vector>

namespace net {

  /**
   * @brief Command structure
   */
  struct Command {
    std::string name;
    std::string description;
    std::function<bool(const std::vector<std::string> &)> handler;
    std::string usage;
  };

  /**
   * @brief Net tool class
   * @details Main class for the net command-line tool
   */
  class NetTool {
  public:
    /**
     * @brief Constructor
     */
    NetTool();

    /**
     * @brief Destructor
     */
    ~NetTool();

    /**
     * @brief Run the tool with command line arguments
     * @param argc Argument count
     * @param argv Argument vector
     * @return Exit code
     */
    int run(int argc, char *argv[]);

    /**
     * @brief Run interactive shell
     * @return Exit code
     */
    int runInteractive();

    /**
     * @brief Execute a single command
     * @param command Command string
     * @return true on success, false on error
     */
    bool executeCommand(const std::string &command);

  private:
    // Interface management
    libfreebsdnet::interface::Manager interfaceManager;
    libfreebsdnet::routing::RoutingTable routingTable;
    libfreebsdnet::netlink::NetlinkManager netlinkManager;

    // Command registry
    std::map<std::string, Command> commands;

    // Internal state
    bool interactive{false};
    std::string prompt{"net> "};

    /**
     * @brief Initialize commands
     */
    void initializeCommands();

    /**
     * @brief Parse command line arguments
     * @param argc Argument count
     * @param argv Argument vector
     * @return true on success, false on error
     */
    bool parseArguments(int argc, char *argv[]);

    /**
     * @brief Show help
     */
    void showHelp();

    /**
     * @brief Show version
     */
    void showVersion();

    // Command handlers
    bool handleShowInterfaces(const std::vector<std::string> &args);
    bool handleShowInterfaceInfo(const std::vector<std::string> &args);
    bool handleShowInterfaceType(const std::vector<std::string> &args);
    bool handleShowInterfaceTypeBridge(const std::vector<std::string> &args);
    bool handleShowInterfaceTypeLagg(const std::vector<std::string> &args);
    bool handleShowInterfaceTypeGif(const std::vector<std::string> &args);
    bool handleShowInterfaceTypeEthernet(const std::vector<std::string> &args);
    bool handleSetInterface(const std::vector<std::string> &args);
    bool handleDeleteInterface(const std::vector<std::string> &args);
    bool handleShowRoute(const std::vector<std::string> &args);
    bool handleShowRouteInfo(const std::vector<std::string> &args);
    bool handleSetRoute(const std::vector<std::string> &args);
    bool handleAddRoute(const std::vector<std::string> &args);
    bool handleDeleteRoute(const std::vector<std::string> &args);
    bool handleFlushRoutes(const std::vector<std::string> &args);
    bool handleShowRouteStats(const std::vector<std::string> &args);
    bool handleShowSystem(const std::vector<std::string> &args);
    bool handleSetSystem(const std::vector<std::string> &args);
    bool handleHelp(const std::vector<std::string> &args);
    bool handleExit(const std::vector<std::string> &args);
    bool handleQuit(const std::vector<std::string> &args);
    bool handleClear(const std::vector<std::string> &args);

    // Utility functions
    std::vector<std::string> splitCommand(const std::string &command);
    void printError(const std::string &message);
    void printSuccess(const std::string &message);
    void printInfo(const std::string &message);
    void printTable(const std::vector<std::vector<std::string>> &data,
                    const std::vector<std::string> &headers);
  };

} // namespace net

#endif // NET_TOOL_HPP
