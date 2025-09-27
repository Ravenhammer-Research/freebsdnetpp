# libfreebsdnet++

A modern C++23 wrapper library for FreeBSD network functionality, providing high-level interfaces for network interface management, packet filtering, routing, traffic management, and high-performance packet I/O.

## Features

### Fully Implemented
- **Network Interface Management**: Query and manage network interfaces
- **Routing**: Network routing table management with multi-FIB support
- **Bridge Management**: Bridge interface creation, configuration, and STP support
- **LAGG Management**: Link aggregation group configuration and protocols
- **VLAN Support**: VLAN interface management and configuration
- **Tunnel Support**: GRE, GIF, TAP, and other tunnel interfaces
- **IPv6 Configuration**: Complete IPv6 options and SLAAC support
- **System Configuration**: System-wide network settings and FIB management

### Under Construction
- **BPF (Berkeley Packet Filter)**: Packet filtering and capture capabilities *(in development)*
- **ALTQ**: Traffic management and quality of service *(planned)*
- **Ethernet**: Ethernet frame handling and MAC address operations *(partial)*
- **Netmap**: High-performance packet I/O for network applications *(planned)*
- **Media Management**: Network media type detection and configuration *(partial)*

## Requirements

- **FreeBSD** operating system
- **C++23** compatible compiler (clang++ or g++)
- **CMake 3.31.6** or later
- **Root privileges** for some operations (BPF, interface management)

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
cmake .. -DBUILD_EXAMPLE=ON
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

**Ethernet Interface** *(Partial Implementation)*
```cpp
class Ethernet : public Interface {
public:
    Ethernet(const std::string& name);
    std::string getMacAddress() const;
    bool setMacAddress(const std::string& mac);  // *(planned)*
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

#### BPF (`libfreebsdnet::bpf`) *(Under Construction)*

> **Note**: BPF functionality is currently under development and not yet fully implemented.

##### `Capture` *(Planned)*
```cpp
class Capture {
public:
    static std::unique_ptr<Capture> create(const std::string& interface);
    
    // Filter management
    bool setFilter(const std::string& filter);
    
    // Capture control
    bool startCapture(std::function<void(const std::vector<uint8_t>&)> callback);
    void stopCapture();
    bool isCapturing() const;
};
```

##### `Filter` *(Planned)*
```cpp
class Filter {
public:
    static std::unique_ptr<Filter> create();
    
    bool compile(const std::string& expression);
    std::vector<uint8_t> getBytecode() const;
};
```

#### Netmap (`libfreebsdnet::netmap`) *(Under Construction)*

> **Note**: Netmap functionality is planned for future implementation to provide high-performance packet I/O.

##### `Interface` *(Planned)*
```cpp
class Interface {
public:
    static std::unique_ptr<Interface> create(const std::string& interface);
    
    // Ring management
    std::shared_ptr<Ring> getRxRing(int index) const;
    std::shared_ptr<Ring> getTxRing(int index) const;
    
    // Statistics
    uint64_t getRxPackets() const;
    uint64_t getTxPackets() const;
    uint64_t getRxBytes() const;
    uint64_t getTxBytes() const;
};
```

##### `Ring` *(Planned)*
```cpp
class Ring {
public:
    // Packet operations
    bool sendPacket(const uint8_t* data, size_t size);
    bool receivePacket(std::vector<uint8_t>& packet);
    
    // Ring state
    bool isEmpty() const;
    bool isFull() const;
    size_t getAvailableSlots() const;
};
```

#### Types (`libfreebsdnet::types`)

##### `Address`
```cpp
class Address {
public:
    Address(const std::string& address);
    Address(const sockaddr* sa);
    
    // Address properties
    AddressFamily getFamily() const;
    std::string toString() const;
    std::string getCidr() const;
    bool isIpv4() const;
    bool isIpv6() const;
    bool isLinkLocal() const;
    
    // Comparison
    bool operator==(const Address& other) const;
};
```

##### `Manager`
```cpp
class Manager {
public:
    static std::unique_ptr<Manager> create();
    
    Address parseAddress(const std::string& address) const;
    std::string formatAddress(const Address& address) const;
};
```

#### System Configuration (`libfreebsdnet::system`)

##### `Config`
```cpp
class Config {
public:
    static std::unique_ptr<Config> create();
    
    // System properties
    int getFibCount() const;
    bool isIpv4ForwardingEnabled() const;
    bool isIpv6ForwardingEnabled() const;
    bool isAddAddrAllfibsEnabled() const;
    
    // Configuration management
    bool setFibCount(int count);
    bool setIpv4Forwarding(bool enabled);
    bool setIpv6Forwarding(bool enabled);
    bool setAddAddrAllfibs(bool enabled);
};
```

## Usage Examples

### Basic Interface Management
```cpp
#include <libfreebsdnet++.hpp>

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
#include <libfreebsdnet++.hpp>

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

### BPF Packet Capture *(Under Construction)*
```cpp
#include <libfreebsdnet++.hpp>

// NOTE: BPF functionality is not yet implemented
// This is a planned API for future development

// Create BPF interface (planned)
// auto bpf = libfreebsdnet::bpf::Capture::create("re0");

// Set filter (planned)
// bpf->setFilter("tcp port 80");

// Start capture (planned)
// bpf->startCapture([](const auto& packet) {
//     std::cout << "Captured packet of size: " << packet.size() << std::endl;
// });
```

### Bridge Management
```cpp
#include <libfreebsdnet++.hpp>

// Create bridge interface
auto bridge = std::make_shared<libfreebsdnet::interface::Bridge>("bridge0");

// Configure bridge
bridge->setStpEnabled(true);
bridge->addMember("em0");
bridge->addMember("em1");

// Bring up bridge
bridge->bringUp();
```

### Netmap High-Performance I/O *(Under Construction)*
```cpp
#include <libfreebsdnet++.hpp>

// NOTE: Netmap functionality is not yet implemented
// This is a planned API for high-performance packet I/O

// Create netmap interface (planned)
// auto netmap = libfreebsdnet::netmap::Interface::create("re0");

// Get transmission ring (planned)
// auto txRing = netmap->getTxRing(0);
// if (txRing) {
//     // Send packets at line rate
//     txRing->sendPacket(packetData, packetSize);
// }
```

### IPv6 Configuration
```cpp
#include <libfreebsdnet++.hpp>

// Get interface and configure IPv6 options
auto iface = manager->getInterface("re0");
if (iface) {
    // Enable IPv6 options
    iface->setIpv6Option(Interface::Ipv6Option::ACCEPT_RTADV, true);
    iface->setIpv6Option(Interface::Ipv6Option::AUTO_LINKLOCAL, true);
    iface->setIpv6Option(Interface::Ipv6Option::PERFORM_NUD, true);
    
    // Add IPv6 address
    iface->addAddress("2001:db8::1/64");
}
```

### Multi-FIB Routing
```cpp
#include <libfreebsdnet++.hpp>

// Configure system for multiple FIBs
auto config = libfreebsdnet::system::Config::create();
config->setFibCount(8);

// Add routes to specific FIBs
auto routingTable = libfreebsdnet::routing::Table::create();
routingTable->addEntry("0.0.0.0", "192.168.1.1", "re0", 0);
routingTable->addEntry("0.0.0.0", "10.1.0.1", "lagg0", 1);
routingTable->addEntry("2000::/3", "%lagg0", "", 6);
```

## Library Architecture

### Namespace Organization
- `libfreebsdnet::interface` - Network interface management
- `libfreebsdnet::routing` - Routing table operations
- `libfreebsdnet::bpf` - Packet filtering and capture
- `libfreebsdnet::netmap` - High-performance packet I/O
- `libfreebsdnet::types` - Common types and utilities
- `libfreebsdnet::system` - System configuration
- `libfreebsdnet::altq` - Traffic management
- `libfreebsdnet::ethernet` - Ethernet operations
- `libfreebsdnet::bridge` - Bridge management
- `libfreebsdnet::vlan` - VLAN support
- `libfreebsdnet::tunnel` - Tunnel interfaces
- `libfreebsdnet::media` - Media management

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

### Component Overview

#### Interface Management (`libfreebsdnet::interface`)
- Query network interfaces with comprehensive metadata
- Manage interface flags, MTU, and configuration
- Support for specialized interface types (Bridge, LAGG, VLAN, etc.)
- IPv6 options and SLAAC configuration
- Address management and validation

#### Routing (`libfreebsdnet::routing`)
- Multi-FIB routing table management
- IPv4 and IPv6 route support with proper netmask handling
- Route flags and metadata extraction
- Gateway resolution and interface association
- Route statistics and monitoring

#### BPF (`libfreebsdnet::bpf`) *(Under Construction)*
- Berkeley Packet Filter compilation and execution *(planned)*
- High-performance packet capture with zero-copy operations *(planned)*
- Complex filter expression support *(planned)*
- Promiscuous mode and interface binding *(planned)*

#### Netmap (`libfreebsdnet::netmap`) *(Under Construction)*
- Wire-speed packet I/O for high-performance applications *(planned)*
- Ring buffer management for transmission and reception *(planned)*
- Zero-copy packet operations *(planned)*
- Statistics and performance monitoring *(planned)*

#### System Configuration (`libfreebsdnet::system`)
- System-wide network configuration management
- Multi-FIB support and configuration
- IP forwarding and routing policies
- System parameter management

### Tunnel Support (`libfreebsdnet::tunnel`)
- GRE, GIF, TAP tunnel interfaces
- Tunnel endpoint configuration
- Tunnel statistics and monitoring

### ALTQ (`libfreebsdnet::altq`) *(Under Construction)*
- Traffic queue management *(planned)*
- Quality of service scheduling *(planned)*
- Bandwidth allocation *(planned)*
- Multiple scheduler types *(planned)*

### Ethernet (`libfreebsdnet::ethernet`) *(Partial Implementation)*
- MAC address operations *(partial)*
- Ethernet frame handling *(planned)*
- Frame validation and checksums *(planned)*
- Protocol type management *(planned)*
- Multiple tunnel types support

### Media Management (`libfreebsdnet::media`) *(Partial Implementation)*
- Network media type detection *(partial)*
- Media option configuration *(planned)*
- Auto-negotiation support
- Media capability queries

### Interface Types (`libfreebsdnet::types`)
- Interface type detection
- Feature capability checking
- Type-specific operations
- Comprehensive type support

## Error Handling

The library uses modern C++ error handling patterns:

```cpp
try {
    libfreebsdnet::interface::Manager manager;
    auto iface = manager.getInterface("eth0");
    if (!iface) {
        throw std::runtime_error("Interface not found");
    }
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## Thread Safety

- **Interface operations**: Thread-safe for read operations
- **BPF capture**: Thread-safe with proper synchronization
- **Netmap operations**: High-performance, lock-free design
- **Statistics collection**: Thread-safe read operations

## Performance Considerations

- **Netmap**: Provides wire-speed packet I/O with minimal overhead
- **BPF**: Efficient packet filtering with compiled bytecode
- **Interface queries**: Cached results for better performance
- **Memory management**: RAII and smart pointers for automatic cleanup

## License

This library is provided as-is for educational and development purposes. Please ensure compliance with FreeBSD licensing and system requirements.

## Contributing

When contributing to this library:

1. Follow the coding conventions in `CONVENTIONS.md`
2. Ensure C++23 compatibility
3. Add comprehensive error handling
4. Include documentation for public APIs
5. Test on FreeBSD systems

## Example Program

Build and run the included example:

```bash
make example
sudo ./example
```

The example demonstrates:
- Interface enumeration
- MAC address operations
- BPF filter compilation
- Statistics collection
- Error handling patterns

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

For complete documentation of the `net` tool, including all commands, examples, and usage patterns, see the [Net Tool README](examples/net/README.md).

## Dependencies

- **System headers**: `/usr/include/net/*`
- **System libraries**: Standard FreeBSD network libraries
- **Build tools**: CMake, C++23 compiler
- **Runtime**: FreeBSD kernel with netmap, BPF, and ALTQ support

## API Reference Summary

### Core Interface Types

| Interface Type | Class | Key Features |
|----------------|-------|--------------|
| **Ethernet** | `Ethernet` | MAC address management, frame handling |
| **Bridge** | `Bridge` | STP support, member management |
| **LAGG** | `Lagg` | Link aggregation, protocol support |
| **VLAN** | `Vlan` | VLAN ID, parent interface management |
| **Loopback** | `Loopback` | Local communication interface |
| **Tunnel** | `Tunnel` | GRE, GIF, TAP tunnel support |
| **Wireless** | `Wireless` | WiFi interface management |

### IPv6 Options

| Option | Description | Default |
|--------|-------------|---------|
| `ACCEPT_RTADV` | Accept Router Advertisements | `true` |
| `PERFORM_NUD` | Perform Neighbor Unreachability Detection | `true` |
| `IFDISABLED` | Interface disabled for IPv6 | `false` |
| `AUTO_LINKLOCAL` | Auto-generate link-local address | `true` |
| `NO_RADR` | No Router Advertisement | `false` |
| `NO_DAD` | No Duplicate Address Detection | `false` |

### LAGG Protocols

| Protocol | Description | Use Case |
|----------|-------------|----------|
| `FAILOVER` | Active/backup failover | High availability |
| `LACP` | Link Aggregation Control Protocol | IEEE 802.3ad standard |
| `LOADBALANCE` | Load balancing | Performance |
| `ROUNDROBIN` | Round-robin distribution | Simple load balancing |

### Route Flags

| Flag | Letter | Description |
|------|--------|-------------|
| `UP` | U | Route is up and active |
| `GATEWAY` | G | Route has a gateway |
| `HOST` | H | Host route (single host) |
| `REJECT` | R | Route is rejected |
| `DYNAMIC` | D | Route was created dynamically |
| `MODIFIED` | M | Route was modified |
| `STATIC` | S | Route is static |
| `BLACKHOLE` | B | Route is a blackhole |

### Error Handling

All library methods return `bool` for success/failure or use exceptions for error conditions. Use `getLastError()` methods to retrieve detailed error messages:

```cpp
auto routingTable = libfreebsdnet::routing::Table::create();
if (!routingTable->addEntry("192.168.1.0/24", "192.168.1.1", "re0")) {
    std::cerr << "Error: " << routingTable->getLastError() << std::endl;
}
```

### Thread Safety

- **Interface Manager**: Thread-safe for read operations
- **Routing Table**: Thread-safe for read operations  
- **System Config**: Thread-safe for read operations
- **BPF Capture**: *(Planned)* Not thread-safe (single-threaded capture)
- **Netmap Interface**: *(Planned)* Thread-safe with proper synchronization

### Performance Considerations

- **Routing**: Efficient routing table queries with minimal overhead
- **Interface Management**: Cached interface information for fast lookups
- **Netmap**: *(Planned)* Wire-speed packet I/O with zero-copy operations
- **BPF**: *(Planned)* Optimized for high-performance packet filtering

### Memory Management

- All objects use smart pointers for automatic memory management
- RAII ensures proper resource cleanup
- No manual memory management required
- Exception-safe resource handling

### Integration with FreeBSD Tools

The library is designed to be compatible with standard FreeBSD networking tools:

- **ifconfig**: Interface configuration compatibility
- **netstat**: Routing table format compatibility  
- **route**: Route management compatibility
- **sysctl**: System parameter compatibility

### Build Integration

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
