# libfreebsdnet++

A modern C++23 wrapper library for FreeBSD network functionality, providing high-level interfaces for network interface management, routing, and system configuration.

## Features

### Core Modules
- **Network Interface Management**: Query and manage network interfaces with comprehensive metadata
- **Routing**: Network routing table management with multi-FIB support
- **Types**: Network address utilities and type-safe interfaces
- **Ethernet**: Ethernet address operations and frame handling
- **Netlink**: Netlink socket communication for advanced networking
- **System**: System-wide network configuration and FIB management

## Requirements

- **FreeBSD** operating system
- **C++23** compatible compiler (clang++ or g++)
- **CMake 3.31.6** or later
- **Root privileges** for some operations (interface management, routing)

## Building

```bash
# Clone or download the library
cd libfreebsdnet++

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the library
make -j $(nproc)

# Build with example (optional)
cmake .. -DBUILD_NET_TOOL=ON
make -j $(nproc)
```

## API Reference

### Core Classes

#### Interface Management (`libfreebsdnet::interface`)

##### `Manager`
```cpp
class Manager {
public:
    static std::unique_ptr<Manager> create();
    std::vector<std::shared_ptr<Interface>> getInterfaces() const;
    std::shared_ptr<Interface> getInterface(const std::string& name) const;
    std::shared_ptr<Interface> getInterface(int index) const;
};
```

##### `Interface` (Base Class)
```cpp
class Interface {
public:
    virtual ~Interface() = default;
    
    // Basic properties
    std::string getName() const;
    InterfaceType getType() const;
    bool isUp() const;
    bool isDown() const;
    int getIndex() const;
    int getMtu() const;
    int getFib() const;
    
    // State management
    bool bringUp();
    bool bringDown();
    
    // Address management
    bool addAddress(const std::string& address);
    bool removeAddress(const std::string& address);
    std::vector<types::Address> getAddresses() const;
    
    // IPv6 options
    enum class Ipv6Option {
        ACCEPT_RTADV,
        PERFORM_NUD,
        IFDISABLED,
        AUTO_LINKLOCAL,
        NO_RADR,
        NO_DAD
    };
    bool setIpv6Option(Ipv6Option option, bool enable);
    bool getIpv6Option(Ipv6Option option) const;
    
    // Configuration
    bool setMtu(int mtu);
    bool setFib(int fib);
    bool setMedia(const std::string& media);
};
```

##### Interface Types

**Ethernet Interface**
```cpp
class Ethernet : public Interface {
public:
    Ethernet(const std::string& name);
    std::string getMacAddress() const;
    bool setMacAddress(const types::Address& address);
};
```

**Bridge Interface**
```cpp
class Bridge : public Interface {
public:
    Bridge(const std::string& name);
    
    // STP management
    bool setStpEnabled(bool enabled);
    bool isStpEnabled() const;
    
    // Member management
    bool addMember(const std::string& interface);
    bool removeMember(const std::string& interface);
    std::vector<std::string> getMembers() const;
};
```

**LAGG Interface**
```cpp
class Lagg : public Interface {
public:
    enum class Protocol {
        FAILOVER,
        LACP,
        LOADBALANCE,
        ROUNDROBIN
    };
    
    Lagg(const std::string& name);
    
    // Protocol management
    bool setProtocol(Protocol protocol);
    Protocol getProtocol() const;
    
    // Member management
    bool addMember(const std::string& interface);
    bool removeMember(const std::string& interface);
    std::vector<std::string> getMembers() const;
};
```

**VLAN Interface**
```cpp
class Vlan : public Interface {
public:
    Vlan(const std::string& name);
    
    bool setVlanId(int vlanId);
    int getVlanId() const;
    bool setParent(const std::string& parent);
    std::string getParent() const;
};
```

#### Routing (`libfreebsdnet::routing`)

##### `Table`
```cpp
class Table {
public:
    static std::unique_ptr<Table> create();
    
    // Route management
    std::vector<std::unique_ptr<RoutingEntry>> getEntries() const;
    bool addEntry(const std::string& destination, 
                  const std::string& gateway,
                  const std::string& interface = "",
                  int fib = 0);
    bool removeEntry(const std::string& destination, int fib = 0);
    
    // Error handling
    std::string getLastError() const;
};
```

##### `RoutingEntry`
```cpp
class RoutingEntry {
public:
    std::string getDestination() const;
    std::string getGateway() const;
    std::string getInterface() const;
    std::string getNetmask() const;
    int getFib() const;
    uint32_t getFlags() const;
    
    // Route flags
    enum class RouteFlag {
        UP, GATEWAY, HOST, REJECT, DYNAMIC, MODIFIED,
        DONE, MASK, CLONING, XRESOLVE, LLINFO, STATIC,
        BLACKHOLE, PROTO2, PROTO1, PRCLONING, WASCLONED,
        PROTO3, PINNED, LOCAL, BROADCAST, MULTICAST
    };
    std::vector<RouteFlag> getFlagList() const;
};
```

#### Types (`libfreebsdnet::types`)

##### `Address`
```cpp
class Address {
public:
    Address(const std::string& address);
    Address(const std::string& ip, int prefixLen);
    
    // Address properties
    std::string getIp() const;
    int getPrefixLength() const;
    Family getFamily() const;
    std::string getNetmask() const;
    std::string getBroadcast() const;
    std::string getCidr() const;
    bool isValid() const;
    bool isIPv4() const;
    bool isIPv6() const;
    
    // Socket address conversion
    struct sockaddr_in getSockaddrIn() const;
    struct sockaddr_in6 getSockaddrIn6() const;
};
```

#### System Configuration (`libfreebsdnet::system`)

##### `SystemConfig`
```cpp
class SystemConfig {
public:
    static std::unique_ptr<SystemConfig> create();
    
    // System properties
    bool getIpForwarding() const;
    bool getIp6Forwarding() const;
    bool getAddAddrAllFibs() const;
    int getFibs() const;
    
    // Configuration management
    bool setIpForwarding(bool enabled);
    bool setIp6Forwarding(bool enabled);
    bool setAddAddrAllFibs(bool enabled);
    bool setFibs(int count);
};
```

#### Netlink (`libfreebsdnet::netlink`)

##### `NetlinkManager`
```cpp
class NetlinkManager {
public:
    NetlinkManager();
    ~NetlinkManager();
    
    // Netlink operations
    bool sendMessage(const std::vector<uint8_t>& message);
    bool receiveMessage(std::vector<uint8_t>& message);
    bool isConnected() const;
};
```

## Usage Examples

### Basic Interface Management
```cpp
#include <interface/lib.hpp>

// Create interface manager
auto manager = libfreebsdnet::interface::Manager::create();

// List all interfaces
auto interfaces = manager->getInterfaces();
for (const auto& iface : interfaces) {
    std::cout << "Interface: " << iface->getName() 
              << " Type: " << iface->getType() << std::endl;
}

// Get specific interface
auto iface = manager->getInterface("re0");
if (iface) {
    std::cout << "Interface " << iface->getName() 
              << " is " << (iface->isUp() ? "UP" : "DOWN") << std::endl;
}
```

### Routing Table Management
```cpp
#include <routing/lib.hpp>

// Create routing table manager
auto routingTable = libfreebsdnet::routing::Table::create();

// Get all routes
auto routes = routingTable->getEntries();
for (const auto& route : routes) {
    std::cout << "Route: " << route->getDestination() 
              << " via " << route->getGateway() << std::endl;
}

// Add a route
if (routingTable->addEntry("192.168.1.0/24", "192.168.1.1", "re0")) {
    std::cout << "Route added successfully" << std::endl;
}
```

### Address Management
```cpp
#include <types/lib.hpp>

// Create address from string
auto addr = libfreebsdnet::types::Address("192.168.1.1/24");
if (addr.isValid()) {
    std::cout << "IP: " << addr.getIp() << std::endl;
    std::cout << "Netmask: " << addr.getNetmask() << std::endl;
    std::cout << "Broadcast: " << addr.getBroadcast() << std::endl;
}
```

### System Configuration
```cpp
#include <system/lib.hpp>

// Get system configuration
auto config = libfreebsdnet::system::SystemConfig::create();
std::cout << "IP Forwarding: " << config->getIpForwarding() << std::endl;
std::cout << "IPv6 Forwarding: " << config->getIp6Forwarding() << std::endl;
std::cout << "Number of FIBs: " << config->getFibs() << std::endl;
```

## Net Tool Example

The project includes a comprehensive network management tool (`net`) that demonstrates the library's capabilities:

```bash
# Build the net tool
cd build/examples/net
make

# Run the net tool
sudo ./net

# Show available commands
net> help

# Execute commands directly
sudo ./net -c "show interface"
sudo ./net -c "show route"
sudo ./net -c "save state" | sudo ./net -c -
```

### Net Tool Features

- **Interface Management**: Query, configure, and manage network interfaces
- **Routing**: View and manage routing tables across multiple FIBs
- **Bridge Management**: Configure bridge interfaces and STP
- **LAGG Management**: Configure link aggregation groups
- **IPv6 Support**: Full IPv6 interface options and routing
- **System Configuration**: Manage system-level network settings
- **Configuration Export**: Generate `net` commands to recreate current state
- **Complete CRUD Operations**: Create, Read, Update, Delete for all network settings
- **Interactive Shell**: Command-line interface with tab completion
- **Batch Mode**: Execute commands via `-c` flag
- **Default Reset**: Restore any configuration to system defaults

## Library Architecture

### Namespace Organization
- `libfreebsdnet::interface` - Network interface management
- `libfreebsdnet::routing` - Routing table operations
- `libfreebsdnet::types` - Common types and utilities
- `libfreebsdnet::ethernet` - Ethernet operations
- `libfreebsdnet::netlink` - Netlink communication
- `libfreebsdnet::system` - System configuration

### Design Principles

#### Modern C++23 Features
- **Smart Pointers**: `std::unique_ptr` and `std::shared_ptr` for automatic memory management
- **RAII**: Resource Acquisition Is Initialization for proper cleanup
- **Move Semantics**: Efficient resource transfer and zero-copy operations
- **Type Safety**: Strong typing with enums and type-safe interfaces
- **Exception Safety**: Proper error handling with exceptions and error codes

#### FreeBSD Integration
- **Native System Calls**: Direct use of FreeBSD system calls and ioctls
- **Kernel Integration**: Deep integration with FreeBSD networking stack
- **Performance**: Optimized for FreeBSD's networking architecture
- **Compatibility**: Full compatibility with FreeBSD networking tools

#### API Design
- **Factory Pattern**: Static `create()` methods for object instantiation
- **Interface Segregation**: Small, focused interfaces for specific functionality
- **Dependency Injection**: Configurable components and dependencies
- **Error Handling**: Consistent error reporting and handling patterns

## Error Handling

The library uses modern C++ error handling patterns:

```cpp
try {
    auto manager = libfreebsdnet::interface::Manager::create();
    auto iface = manager->getInterface("eth0");
    if (!iface) {
        throw std::runtime_error("Interface not found");
    }
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## Thread Safety

- **Interface operations**: Thread-safe for read operations
- **Routing Table**: Thread-safe for read operations  
- **System Config**: Thread-safe for read operations
- **Netlink Manager**: Thread-safe with proper synchronization

## Performance Considerations

- **Routing**: Efficient routing table queries with minimal overhead
- **Interface Management**: Cached interface information for fast lookups
- **Memory management**: RAII and smart pointers for automatic cleanup
- **Zero-copy operations**: Move semantics for efficient resource transfer

## Memory Management

- All objects use smart pointers for automatic memory management
- RAII ensures proper resource cleanup
- No manual memory management required
- Exception-safe resource handling

## Integration with FreeBSD Tools

The library is designed to be compatible with standard FreeBSD networking tools:

- **ifconfig**: Interface configuration compatibility
- **netstat**: Routing table format compatibility  
- **route**: Route management compatibility
- **sysctl**: System parameter compatibility

## Build Integration

```cmake
# Find the library
find_package(libfreebsdnet++ REQUIRED)

# Link against the library
target_link_libraries(your_target libfreebsdnet++)

# Include headers
target_include_directories(your_target PRIVATE ${libfreebsdnet++_INCLUDE_DIRS})
```

### Example CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.31.6)
project(my_network_app)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libfreebsdnet++ REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app libfreebsdnet++)
```

## Dependencies

- **System headers**: `/usr/include/net/*`
- **System libraries**: Standard FreeBSD network libraries
- **Build tools**: CMake, C++23 compiler
- **Runtime**: FreeBSD kernel with networking support

## License

This library is provided as-is for educational and development purposes. Please ensure compliance with FreeBSD licensing and system requirements.

## Contributing

When contributing to this library:

1. Follow the coding conventions in `CONVENTIONS.md`
2. Ensure C++23 compatibility
3. Add comprehensive error handling
4. Include documentation for public APIs
5. Test on FreeBSD systems