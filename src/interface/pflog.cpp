/**
 * @file interface/pflog.cpp
 * @brief PFLOG interface implementation
 * @details Implementation of PFLOG interface functionality
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <ifaddrs.h>
#include <interface/pflog.hpp>
#include <net/if.h>
#include <net/if_mib.h>
#include <net/if_pflog.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace libfreebsdnet::interface {

  class PflogInterface::Impl {
  public:
    std::string name;
    unsigned int index;
    int flags;
    std::string lastError;
    std::string ruleset;
    int ruleNumber;
    bool enabled;

    Impl(const std::string &name, unsigned int index, int flags)
        : name(name), index(index), flags(flags), ruleNumber(0),
          enabled(false) {}
  };

  PflogInterface::PflogInterface(const std::string &name, unsigned int index,
                                 int flags)
      : pImpl(std::make_unique<Impl>(name, index, flags)) {}

  PflogInterface::~PflogInterface() = default;

  // Base class method implementations
  std::string PflogInterface::getName() const { return Interface::getName(); }
  unsigned int PflogInterface::getIndex() const { return Interface::getIndex(); }
  InterfaceType PflogInterface::getType() const { return InterfaceType::PFLOG; }
  std::vector<Flag> PflogInterface::getFlags() const { return Interface::getFlags(); }
  bool PflogInterface::setFlags(int flags) { return Interface::setFlags(flags); }
  bool PflogInterface::bringUp() { return Interface::bringUp(); }
  bool PflogInterface::bringDown() { return Interface::bringDown(); }
  bool PflogInterface::isUp() const { return Interface::isUp(); }

  int PflogInterface::getMtu() const { return Interface::getMtu(); }
  bool PflogInterface::setMtu(int mtu) { return Interface::setMtu(mtu); }
  std::string PflogInterface::getLastError() const { return Interface::getLastError(); }
  int PflogInterface::getFib() const { return Interface::getFib(); }
  bool PflogInterface::setFib(int fib) { return Interface::setFib(fib); }

  std::string PflogInterface::getLogInterface() const { return pImpl->ruleset; }

  bool PflogInterface::setLogInterface(const std::string &interfaceName) {
    pImpl->ruleset = interfaceName;
    return true; // Interface setting would require specific PFLOG ioctls
  }

  int PflogInterface::getLogRule() const { return pImpl->ruleNumber; }

  bool PflogInterface::setLogRule(int ruleNumber) {
    pImpl->ruleNumber = ruleNumber;
    return true; // Rule number setting would require specific PFLOG ioctls
  }


  int PflogInterface::getMedia() const {
    return Interface::getMedia();
  }

  bool PflogInterface::setMedia(int media) {
    return Interface::setMedia(media);
  }

  int PflogInterface::getMediaStatus() const {
    return Interface::getMediaStatus();
  }

  int PflogInterface::getActiveMedia() const {
    return Interface::getActiveMedia();
  }

  std::vector<int> PflogInterface::getSupportedMedia() const {
    return Interface::getSupportedMedia();
  }

  uint32_t PflogInterface::getCapabilities() const {
    return Interface::getCapabilities();
  }

  bool PflogInterface::setCapabilities(uint32_t capabilities) {
    return Interface::setCapabilities(capabilities);
  }

  uint32_t PflogInterface::getEnabledCapabilities() const {
    return Interface::getEnabledCapabilities();
  }

  bool PflogInterface::enableCapabilities(uint32_t capabilities) {
    return Interface::enableCapabilities(capabilities);
  }

  bool PflogInterface::disableCapabilities(uint32_t capabilities) {
    return Interface::disableCapabilities(capabilities);
  }

  std::vector<std::string> PflogInterface::getGroups() const {
    return Interface::getGroups();
  }

  bool PflogInterface::addToGroup(const std::string &groupName) {
    return Interface::addToGroup(groupName);
  }

  bool PflogInterface::removeFromGroup(const std::string &groupName) {
    return Interface::removeFromGroup(groupName);
  }


  bool PflogInterface::setPhysicalAddress(const std::string &address) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifaliasreq ifra;
    std::memset(&ifra, 0, sizeof(ifra));
    std::strncpy(ifra.ifra_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    // Parse IP address
    struct sockaddr_in *sin =
        reinterpret_cast<struct sockaddr_in *>(&ifra.ifra_addr);
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, address.c_str(), &sin->sin_addr) != 1) {
      pImpl->lastError = "Invalid IP address format";
      close(sock);
      return false;
    }

    if (ioctl(sock, SIOCSIFPHYADDR, &ifra) < 0) {
      pImpl->lastError =
          "Failed to set physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool PflogInterface::deletePhysicalAddress() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCDIFPHYADDR, &ifr) < 0) {
      pImpl->lastError =
          "Failed to delete physical address: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  bool PflogInterface::createClone(const std::string &cloneName) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket";
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, cloneName.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
      pImpl->lastError =
          "Failed to create clone: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }

  std::vector<std::string> PflogInterface::getCloners() const {
    std::vector<std::string> cloners;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      return cloners;
    }

    struct if_clonereq ifcr;
    std::memset(&ifcr, 0, sizeof(ifcr));

    // First get the total number of cloners
    if (ioctl(sock, SIOCIFGCLONERS, &ifcr) < 0) {
      close(sock);
      return cloners;
    }

    if (ifcr.ifcr_total > 0) {
      // Allocate buffer for cloner names
      std::vector<char> buffer(ifcr.ifcr_total * IFNAMSIZ);
      ifcr.ifcr_buffer = buffer.data();
      ifcr.ifcr_count = ifcr.ifcr_total;

      // Get the cloner names
      if (ioctl(sock, SIOCIFGCLONERS, &ifcr) == 0) {
        for (int i = 0; i < ifcr.ifcr_count; i++) {
          std::string cloner(buffer.data() + (i * IFNAMSIZ));
          if (!cloner.empty()) {
            cloners.push_back(cloner);
          }
        }
      }
    }

    close(sock);
    return cloners;
  }

  std::string PflogInterface::getMacAddress() const {
    // PFLOG interfaces typically don't have MAC addresses
    return "";
  }

  bool PflogInterface::setMacAddress(const std::string &macAddress) {
    (void)macAddress; // Suppress unused parameter warning
    // PFLOG interfaces typically don't support MAC addresses
    pImpl->lastError = "PFLOG interfaces do not support MAC addresses";
    return false;
  }


  bool PflogInterface::destroy() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      pImpl->lastError = "Failed to create socket: " + std::string(strerror(errno));
      return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, pImpl->name.c_str(), IFNAMSIZ - 1);

    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
      pImpl->lastError = "Failed to destroy interface: " + std::string(strerror(errno));
      close(sock);
      return false;
    }

    close(sock);
    return true;
  }


} // namespace libfreebsdnet::interface
