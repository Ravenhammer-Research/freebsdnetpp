/**
 * @file system/config.cpp
 * @brief System network configuration implementation
 * @details Implementation of system network configuration via sysctl
 *
 * @author paigeadelethompson
 * @year 2024
 */

#include <system/config.hpp>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <cstring>
#include <sstream>

namespace libfreebsdnet::system {

  class SystemConfig::Impl {
  public:
    std::string lastError;

    Impl() : lastError("") {}

    template<typename T>
    T getSysctlValue(const char *name, T defaultValue) const {
      size_t size = sizeof(T);
      T value = defaultValue;
      
      int mib[CTL_MAXNAME];
      size_t miblen = CTL_MAXNAME;
      
      if (sysctlnametomib(name, mib, &miblen) < 0) {
        return defaultValue;
      }
      
      if (sysctl(mib, miblen, &value, &size, nullptr, 0) < 0) {
        return defaultValue;
      }
      
      return value;
    }

    std::string getSysctlString(const char *name, const std::string &defaultValue = "") const {
      size_t size = 0;
      
      int mib[CTL_MAXNAME];
      size_t miblen = CTL_MAXNAME;
      
      if (sysctlnametomib(name, mib, &miblen) < 0) {
        return defaultValue;
      }
      
      // Get required buffer size
      if (sysctl(mib, miblen, nullptr, &size, nullptr, 0) < 0) {
        return defaultValue;
      }
      
      if (size == 0) {
        return defaultValue;
      }
      
      std::string result(size - 1, '\0'); // -1 to exclude null terminator
      
      if (sysctl(mib, miblen, &result[0], &size, nullptr, 0) < 0) {
        return defaultValue;
      }
      
      return result;
    }

    int getFibs() const {
      return getSysctlValue("net.fibs", 1);
    }

    bool getAddAddrAllFibs() const {
      return getSysctlValue("net.add_addr_allfibs", 0) != 0;
    }

    bool getIpForwarding() const {
      return getSysctlValue("net.inet.ip.forwarding", 0) != 0;
    }

    bool getIp6Forwarding() const {
      return getSysctlValue("net.inet6.ip6.forwarding", 0) != 0;
    }

    bool getRouteMultipath() const {
      return getSysctlValue("net.route.multipath", 0) != 0;
    }

    bool getRouteHashOutbound() const {
      return getSysctlValue("net.route.hash_outbound", 0) != 0;
    }

    bool getRouteIpv6Nexthop() const {
      return getSysctlValue("net.route.ipv6_nexthop", 0) != 0;
    }

    std::string getRouteInetAlgo() const {
      return getSysctlString("net.route.algo.inet.algo", "unknown");
    }

    std::string getRouteInet6Algo() const {
      return getSysctlString("net.route.algo.inet6.algo", "unknown");
    }

    int getNetisrMaxqlen() const {
      return getSysctlValue("net.route.netisr_maxqlen", 256);
    }

    int getFibMaxSyncDelay() const {
      return getSysctlValue("net.route.algo.fib_max_sync_delay_ms", 1000);
    }

    std::map<std::string, std::string> getAllConfig() const {
      std::map<std::string, std::string> config;
      
      config["net.fibs"] = std::to_string(getFibs());
      config["net.add_addr_allfibs"] = getAddAddrAllFibs() ? "1" : "0";
      config["net.inet.ip.forwarding"] = getIpForwarding() ? "1" : "0";
      config["net.inet6.ip6.forwarding"] = getIp6Forwarding() ? "1" : "0";
      config["net.route.multipath"] = getRouteMultipath() ? "1" : "0";
      config["net.route.hash_outbound"] = getRouteHashOutbound() ? "1" : "0";
      config["net.route.ipv6_nexthop"] = getRouteIpv6Nexthop() ? "1" : "0";
      config["net.route.algo.inet.algo"] = getRouteInetAlgo();
      config["net.route.algo.inet6.algo"] = getRouteInet6Algo();
      config["net.route.netisr_maxqlen"] = std::to_string(getNetisrMaxqlen());
      config["net.route.algo.fib_max_sync_delay_ms"] = std::to_string(getFibMaxSyncDelay());
      
      return config;
    }
  };

  SystemConfig::SystemConfig() : pImpl(std::make_unique<Impl>()) {}

  SystemConfig::~SystemConfig() = default;

  int SystemConfig::getFibs() const {
    return pImpl->getFibs();
  }

  bool SystemConfig::getAddAddrAllFibs() const {
    return pImpl->getAddAddrAllFibs();
  }

  bool SystemConfig::getIpForwarding() const {
    return pImpl->getIpForwarding();
  }

  bool SystemConfig::getIp6Forwarding() const {
    return pImpl->getIp6Forwarding();
  }

  bool SystemConfig::getRouteMultipath() const {
    return pImpl->getRouteMultipath();
  }

  bool SystemConfig::getRouteHashOutbound() const {
    return pImpl->getRouteHashOutbound();
  }

  bool SystemConfig::getRouteIpv6Nexthop() const {
    return pImpl->getRouteIpv6Nexthop();
  }

  std::string SystemConfig::getRouteInetAlgo() const {
    return pImpl->getRouteInetAlgo();
  }

  std::string SystemConfig::getRouteInet6Algo() const {
    return pImpl->getRouteInet6Algo();
  }

  int SystemConfig::getNetisrMaxqlen() const {
    return pImpl->getNetisrMaxqlen();
  }

  int SystemConfig::getFibMaxSyncDelay() const {
    return pImpl->getFibMaxSyncDelay();
  }

  std::map<std::string, std::string> SystemConfig::getAllConfig() const {
    return pImpl->getAllConfig();
  }

  std::string SystemConfig::getLastError() const {
    return pImpl->lastError;
  }

} // namespace libfreebsdnet::system
