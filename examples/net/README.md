# Net Tool

A comprehensive network management tool built with libfreebsdnet++ that provides a command-line interface for managing FreeBSD network interfaces, routing tables, bridges, LAGGs, and system configuration.

## Features

- **Interface Management**: Query, configure, and manage network interfaces
- **Routing**: View and manage routing tables across multiple FIBs
- **Bridge Management**: Configure bridge interfaces and STP
- **LAGG Management**: Configure link aggregation groups
- **IPv6 Support**: Full IPv6 interface options and routing
- **Configuration Export**: Generate `net` commands to recreate current state
- **Interactive Shell**: Command-line interface with tab completion
- **Batch Mode**: Execute commands via `-c` flag

## Building

```bash
# From the project root
cd build/examples/net
make

# Or from the project root
make -C build examples/net
```

## Usage

### Interactive Mode

```bash
sudo ./net
net> help
net> show interface
net> show route
net> save state
net> exit
```

### Batch Mode

```bash
# Execute single commands
sudo ./net -c "show interface"
sudo ./net -c "show route fib 6"
sudo ./net -c "set interface lagg0 state up"

# Save and restore configuration
sudo ./net -c "save state" > config.txt
sudo ./net -c - < config.txt
```

## Commands

### Interface Commands

#### Show Interface Information
```bash
# Show all interfaces
show interface

# Show specific interface
show interface lagg0

# Show interface properties
show interface lagg0 fib
show interface lagg0 mtu
show interface lagg0 media
show interface lagg0 capabilities
show interface lagg0 groups
show interface lagg0 mac
```

#### Set Interface Properties
```bash
# Bring interface up/down
set interface lagg0 state up
set interface lagg0 state down

# Set MTU
set interface lagg0 mtu 9000

# Set FIB
set interface lagg0 fib 6

# Add addresses
set interface lagg0 address 192.168.1.100/24
set interface lagg0 address fe80::1/64

# IPv6 options
set interface lagg0 accept_rtadv enable
set interface lagg0 auto_linklocal enable
set interface lagg0 perform_nud enable
set interface lagg0 slaac enable
set interface lagg0 ifdisabled disable
```

#### Delete Interface Properties
```bash
# Remove addresses
delete interface lagg0 address 192.168.1.100/24

# Disable IPv6 options
delete interface lagg0 ipv6 accept_rtadv
delete interface lagg0 ipv6 ifdisabled
```

### Routing Commands

#### Show Routes
```bash
# Show all routes (default FIB)
show route

# Show routes for specific FIB
show route fib 6

# Show route statistics
show route stats
show route stats fib 6
```

#### Add Routes
```bash
# Add IPv4 route
set route 192.168.100.0/24 10.1.0.1 re0.25

# Add IPv6 route
set route 2000::/3 %lagg0 fib 6

# Add route with interface
add route 10.0.0.0/8 192.168.1.1 lagg0
```

#### Delete Routes
```bash
# Delete route
delete route 192.168.100.0/24
delete route 2000::/3 fib 6
```

### Bridge Commands

#### Show Bridge Information
```bash
# Show all bridges
show interface type bridge

# Show specific bridge
show interface bridge0
```

#### Configure Bridge
```bash
# Enable/disable STP
set bridge bridge0 stp enable
set bridge bridge0 stp disable

# Add/remove members
set bridge bridge0 addm epair1a
set bridge bridge0 delm epair1a
```

### LAGG Commands

#### Show LAGG Information
```bash
# Show all LAGGs
show interface type lagg

# Show specific LAGG
show interface lagg0
```

#### Configure LAGG
```bash
# Set protocol
set lagg lagg0 protocol lacp
set lagg lagg0 protocol failover
set lagg lagg0 protocol loadbalance

# Add/remove members
set lagg lagg0 addm epair4a
set lagg lagg0 delm epair4a
```

### System Commands

#### Show System Configuration
```bash
# Show system network configuration
show system
```

### Save State Command

#### Export Configuration
```bash
# Save current network state
save state

# Save to file
save state > network_config.txt

# Restore from file
net -c - < network_config.txt
```

The `save state` command generates `net` commands that can recreate the current network configuration, including:
- System configuration (IP forwarding, FIB count)
- Interface states (up/down, MTU, FIB, addresses)
- Bridge configuration (STP, members)
- LAGG configuration (protocol, members)
- Routing table (all FIBs)

## Examples

### Basic Interface Management
```bash
# List all interfaces
net> show interface

# Bring up an interface
net> set interface re0 state up

# Set MTU
net> set interface re0 mtu 9000

# Add IP address
net> set interface re0 address 192.168.1.100/24
```

### LAGG Configuration
```bash
# Create LAGG interface
net> set lagg lagg0 protocol lacp
net> set lagg lagg0 addm em0
net> set lagg lagg0 addm em1
net> set interface lagg0 state up
net> set interface lagg0 address 192.168.1.100/24
```

### Bridge Configuration
```bash
# Create bridge
net> set bridge bridge0 stp enable
net> set bridge bridge0 addm em0
net> set bridge bridge0 addm em1
net> set interface bridge0 state up
```

### IPv6 Configuration
```bash
# Enable IPv6 autoconfiguration
net> set interface re0 slaac enable
net> set interface re0 accept_rtadv enable
net> set interface re0 auto_linklocal enable

# Add IPv6 address
net> set interface re0 address 2001:db8::1/64
```

### Routing Configuration
```bash
# Add default route
net> set route 0.0.0.0 192.168.1.1 re0

# Add route to specific FIB
net> set route 10.0.0.0/8 192.168.1.1 re0 fib 1

# Add IPv6 route
net> set route 2000::/3 %re0 fib 6
```

### Configuration Backup and Restore
```bash
# Backup current configuration
net> save state > backup.txt

# Restore configuration
net -c - < backup.txt

# Or in interactive mode
net> # (paste saved commands)
```

## Error Handling

The tool provides clear error messages for common issues:

```bash
# Interface not found
Error: Interface not found: eth0

# Invalid command
Error: Unknown command: invalid

# Permission denied (run with sudo)
Error: Operation not permitted
```

## Requirements

- **FreeBSD** operating system
- **Root privileges** for most operations
- **libfreebsdnet++** library
- **Readline** library for interactive mode

## Limitations

- Some operations require root privileges
- IPv6 route addition with interface names needs `%` prefix
- FIB-specific operations may not work on all FreeBSD versions
- Some interface types may not support all operations

## Troubleshooting

### Common Issues

1. **Permission denied**: Run with `sudo`
2. **Interface not found**: Check interface name with `show interface`
3. **Invalid argument**: Check command syntax with `help`
4. **Route addition fails**: Ensure gateway is reachable

### Debug Mode

For debugging, you can examine the generated commands:

```bash
# Show what commands would be generated
net> save state | head -20
```

## Contributing

When adding new commands or features:

1. Follow the existing command structure
2. Add proper error handling
3. Update this README
4. Test with various interface types
5. Ensure compatibility with different FreeBSD versions
