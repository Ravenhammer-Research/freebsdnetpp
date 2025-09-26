# libfreebsdnet++

A modern C++23 wrapper library for FreeBSD network functionality, providing high-level interfaces for network interface management, packet filtering, routing, traffic management, and high-performance packet I/O.

## Features

- **Network Interface Management**: Query and manage network interfaces
- **BPF (Berkeley Packet Filter)**: Packet filtering and capture capabilities
- **Routing**: Network routing table management
- **ALTQ**: Traffic management and quality of service
- **Ethernet**: Ethernet frame handling and MAC address operations
- **Netmap**: High-performance packet I/O for network applications
- **Bridge Management**: Bridge interface creation, configuration, and STP support
- **VLAN Support**: VLAN interface management and configuration
- **Tunnel Support**: GRE, GIF, TAP, and other tunnel interfaces
- **Media Management**: Network media type detection and configuration
- **Interface Types**: Comprehensive interface type management

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

## Usage

### Basic Interface Management

```cpp
#include <libfreebsdnet++.hpp>
#include <iostream>

int main() {
    // Create interface manager
    libfreebsdnet::interface::Manager manager;
    
    // Get all interfaces
    auto interfaces = manager.getInterfaces();
    
    for (const auto& iface : interfaces) {
        std::cout << "Interface: " << iface.name 
                  << " (index: " << iface.index << ")" << std::endl;
    }
    
    return 0;
}
```

### BPF Packet Filtering

```cpp
#include <libfreebsdnet++.hpp>

// Create BPF filter
libfreebsdnet::bpf::Filter filter;
filter.compile("tcp port 80");

// Use filter builder for complex filters
auto httpFilter = libfreebsdnet::bpf::FilterBuilder()
    .protocol("tcp")
    .dstPort(80)
    .build();

// Packet capture
libfreebsdnet::bpf::PacketCapture capture;
capture.open("eth0", 65536);
capture.setFilter(httpFilter);

capture.startCapture([](const auto& packet) {
    std::cout << "Captured packet: " << packet.length << " bytes" << std::endl;
    return true; // Continue capture
});
```

### Ethernet Operations

```cpp
#include <libfreebsdnet++.hpp>

// MAC address operations
libfreebsdnet::ethernet::MacAddress mac("aa:bb:cc:dd:ee:ff");
std::cout << "MAC: " << mac.toString() << std::endl;

// Create ethernet frame
libfreebsdnet::ethernet::MacAddress dest("ff:ff:ff:ff:ff:ff");
libfreebsdnet::ethernet::MacAddress src = mac;
std::vector<uint8_t> payload = {0x48, 0x65, 0x6c, 0x6c, 0x6f}; // "Hello"

libfreebsdnet::ethernet::Frame frame(dest, src, 
    libfreebsdnet::ethernet::FrameType::IPv4, payload);
```

### High-Performance Packet I/O with Netmap

```cpp
#include <libfreebsdnet++.hpp>

// Configure netmap interface
libfreebsdnet::netmap::NetmapConfig config("eth0", 1, 1);
libfreebsdnet::netmap::NetmapInterface netmap(config);

// Start packet capture
netmap.startCapture([](const uint8_t* packet, uint32_t length) {
    // Process packet at wire speed
    return true;
});

// Send packets
std::vector<uint8_t> packet = {0x48, 0x65, 0x6c, 0x6c, 0x6f};
netmap.sendPacket(packet);
```

## Library Components

### Interface Management (`libfreebsdnet::interface`)
- Query network interfaces
- Manage interface flags
- Collect interface statistics
- Bring interfaces up/down

### BPF (`libfreebsdnet::bpf`)
- Compile packet filters
- Capture network packets
- Filter builder for complex expressions
- Promiscuous mode support

### Routing (`libfreebsdnet::routing`)
- Manage routing tables
- Add/remove routes
- Query routing entries
- Default gateway management

### ALTQ (`libfreebsdnet::altq`)
- Traffic queue management
- Quality of service scheduling
- Bandwidth allocation
- Multiple scheduler types

### Ethernet (`libfreebsdnet::ethernet`)
- MAC address operations
- Ethernet frame handling
- Frame validation and checksums
- Protocol type management

### Netmap (`libfreebsdnet::netmap`)
- High-performance packet I/O
- Zero-copy operations
- Ring buffer management
- Wire-speed packet processing

### Bridge Management (`libfreebsdnet::bridge`)
- Bridge interface creation and management
- Bridge port configuration
- Spanning Tree Protocol (STP) support
- Bridge statistics and monitoring

### VLAN Support (`libfreebsdnet::vlan`)
- VLAN interface creation and management
- VLAN configuration and statistics
- Parent interface management
- VLAN ID validation

### Tunnel Support (`libfreebsdnet::tunnel`)
- GRE, GIF, TAP tunnel interfaces
- Tunnel endpoint configuration
- Tunnel statistics and monitoring
- Multiple tunnel types support

### Media Management (`libfreebsdnet::media`)
- Network media type detection
- Media option configuration
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

## Dependencies

- **System headers**: `/usr/include/net/*`
- **System libraries**: Standard FreeBSD network libraries
- **Build tools**: CMake, C++23 compiler
- **Runtime**: FreeBSD kernel with netmap, BPF, and ALTQ support
