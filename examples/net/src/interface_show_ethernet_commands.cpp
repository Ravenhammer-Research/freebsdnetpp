/**
 * @file interface_show_ethernet_commands.cpp
 * @brief Net tool Ethernet interface show command implementations
 * @details Implementation of Ethernet interface show-related command handlers
 * for the net tool
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <interface/ethernet.hpp>
#include <iostream>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net_tool.hpp>
#include <sstream>
#include <system/config.hpp>

namespace net {

  bool NetTool::handleShowInterfaceTypeEthernet(
      const std::vector<std::string> &args) {
    (void)args; // Suppress unused parameter warning

    auto interfaces = interfaceManager.getInterfaces();

    // Filter for Ethernet interfaces
    std::vector<std::unique_ptr<libfreebsdnet::interface::Interface>>
        ethernetInterfaces;
    for (auto &interface : interfaces) {
      if (interface->getType() ==
          libfreebsdnet::interface::InterfaceType::ETHERNET) {
        ethernetInterfaces.push_back(std::move(interface));
      }
    }

    if (ethernetInterfaces.empty()) {
      printInfo("No Ethernet interfaces found.");
      return true;
    }

    printInfo("Ethernet Interfaces");
    printInfo("===================");
    printInfo("");

    // Prepare table data
    std::vector<std::vector<std::string>> data;
    std::vector<std::string> headers = {
        "Interface", "Status", "MTU", "FIB", "MAC Address", "Media", "Options"};

    for (const auto &interface : ethernetInterfaces) {
      auto ethernetIface =
          dynamic_cast<libfreebsdnet::interface::EthernetInterface *>(
              interface.get());
      if (!ethernetIface) {
        continue;
      }

      // Get capabilities as enum list
      auto capabilities = ethernetIface->getCapabilityList();
      std::vector<std::string> capabilityList;
      if (capabilities.empty()) {
        capabilityList.push_back("None");
      } else {
        for (const auto &cap : capabilities) {
          switch (cap) {
          case libfreebsdnet::interface::Capability::RXCSUM:
            capabilityList.push_back("RXCSUM");
            break;
          case libfreebsdnet::interface::Capability::TXCSUM:
            capabilityList.push_back("TXCSUM");
            break;
          case libfreebsdnet::interface::Capability::VLAN_MTU:
            capabilityList.push_back("VLAN_MTU");
            break;
          case libfreebsdnet::interface::Capability::VLAN_HWTAGGING:
            capabilityList.push_back("VLAN_HWTAGGING");
            break;
          case libfreebsdnet::interface::Capability::VLAN_HWCSUM:
            capabilityList.push_back("VLAN_HWCSUM");
            break;
          case libfreebsdnet::interface::Capability::WOL_MAGIC:
            capabilityList.push_back("WOL_MAGIC");
            break;
          case libfreebsdnet::interface::Capability::LINKSTATE:
            capabilityList.push_back("LINKSTATE");
            break;
          case libfreebsdnet::interface::Capability::TSO4:
            capabilityList.push_back("TSO4");
            break;
          case libfreebsdnet::interface::Capability::TSO6:
            capabilityList.push_back("TSO6");
            break;
          case libfreebsdnet::interface::Capability::LRO:
            capabilityList.push_back("LRO");
            break;
          }
        }
      }

      // Get MAC address
      std::string macAddress = ethernetIface->getMacAddress();
      if (macAddress.empty()) {
        macAddress = "Unknown";
      }

      // Get media information and format it
      auto mediaInfo = ethernetIface->getMediaInfo();
      std::stringstream mediaStr;

      // Media type
      switch (mediaInfo.type) {
      case libfreebsdnet::interface::MediaType::ETHERNET:
        mediaStr << "Ethernet";
        break;
      default:
        mediaStr << "Unknown";
        break;
      }

      // Check for autoselect mode
      bool hasAutoselect = false;
      for (const auto &option : mediaInfo.options) {
        if (option == libfreebsdnet::interface::MediaOption::AUTO_SELECT) {
          hasAutoselect = true;
          break;
        }
      }
      if (hasAutoselect) {
        mediaStr << " autoselect";
      }

      // Media subtype
      std::string subtypeStr;
      switch (mediaInfo.subtype) {
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_10_T:
        subtypeStr = "10baseT";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_10_2:
        subtypeStr = "10base2";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_10_5:
        subtypeStr = "10base5";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_100_TX:
        subtypeStr = "100baseTX";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_100_FX:
        subtypeStr = "100baseFX";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_1000_T:
        subtypeStr = "1000baseT";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_1000_SX:
        subtypeStr = "1000baseSX";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_1000_LX:
        subtypeStr = "1000baseLX";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_10G_T:
        subtypeStr = "10GbaseT";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_10G_SR:
        subtypeStr = "10GbaseSR";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_10G_LR:
        subtypeStr = "10GbaseLR";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_2500_T:
        subtypeStr = "2500baseT";
        break;
      case libfreebsdnet::interface::MediaSubtype::ETHERNET_5000_T:
        subtypeStr = "5000baseT";
        break;
      default:
        subtypeStr = "unknown";
        break;
      }

      // Add subtype in parentheses
      mediaStr << " (" << subtypeStr;

      // Media options
      std::vector<std::string> optionStrs;
      for (const auto &option : mediaInfo.options) {
        switch (option) {
        case libfreebsdnet::interface::MediaOption::FULL_DUPLEX:
          optionStrs.push_back("full-duplex");
          break;
        case libfreebsdnet::interface::MediaOption::HALF_DUPLEX:
          optionStrs.push_back("half-duplex");
          break;
        case libfreebsdnet::interface::MediaOption::AUTO_SELECT:
          break; // Already handled above
        }
      }

      // Add options in angle brackets
      if (!optionStrs.empty()) {
        mediaStr << " <";
        for (size_t i = 0; i < optionStrs.size(); i++) {
          if (i > 0)
            mediaStr << ",";
          mediaStr << optionStrs[i];
        }
        mediaStr << ">";
      }

      // Close parentheses
      mediaStr << ")";

      std::string mediaInfoStr = mediaStr.str();

      // Create rows for each capability
      for (size_t i = 0; i < capabilityList.size(); i++) {
        std::vector<std::string> row;
        if (i == 0) {
          // First row includes all other columns
          row.push_back(interface->getName());
          row.push_back(interface->isUp() ? "UP" : "DOWN");
          row.push_back(std::to_string(interface->getMtu()));
          row.push_back(std::to_string(interface->getFib()));
          row.push_back(macAddress);
          row.push_back(mediaInfoStr);
          row.push_back(capabilityList[i]);
        } else {
          // Subsequent rows only show capability, with empty strings for other
          // columns
          row.push_back("");
          row.push_back("");
          row.push_back("");
          row.push_back("");
          row.push_back("");
          row.push_back("");
          row.push_back(capabilityList[i]);
        }
        data.push_back(row);
      }
    }

    printTable(data, headers);
    return true;
  }

} // namespace net
