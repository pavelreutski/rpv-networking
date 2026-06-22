# POSIX Networking Toolset

A lightweight networking toolset implemented in C using POSIX APIs.

The project provides reusable TCP networking components and reference
applications built on top of them, targeting Unix-like operating systems
with POSIX thread and socket support.

## Features

- POSIX-compliant socket interfaces
- TCP server framework
- Opaque connection handles
- Thread-based server execution
- Graceful server shutdown
- Blocking I/O model
- Minimal dependencies
- Portable C implementation

## Components

### TCP Server

The TCP server module provides a simple callback-based interface for
accepting and handling client connections.

Features:

- TCP listening socket management
- Client connection acceptance
- Connection read/write utilities
- Client endpoint information retrieval
- Server lifecycle management

### Echo Server

A reference application demonstrating usage of the TCP server module.

Features:

- Accepts incoming TCP connections
- Echoes received data back to the client
- Graceful shutdown support

## Requirements

- POSIX-compliant operating system
- C compiler with C23 or later support
- GCC 14 or newer
- GNU Binutils
- Build Essentials
- CMake
- POSIX threads (`pthread`)
- POSIX sockets

Tested on:

- Linux

## Building

Example build command(s):

```sh
make clean && make debug && make release
````

## Design Principles

* Simplicity over abstraction
* Explicit resource ownership
* Opaque public interfaces
* Minimal heap allocation
* Blocking I/O by default
* POSIX-first implementation

## Threading Model

The TCP server executes an internal server thread responsible for:

1. Accepting incoming connections
2. Invoking the application callback
3. Closing the client connection
4. Continuing to accept subsequent connections

Client connections are processed sequentially.

## Connection Lifetime

Connection handles supplied to application callbacks are owned by the
TCP server implementation.

A connection handle:

* Is valid only during callback execution
* Must not be retained after callback return
* Is automatically closed when the callback completes

## License

See project license for details.