# SD/OS Chaos Core V18.0

## Project Description
SD/OS Chaos Core V18.0 is a bare-metal kernel simulation featuring a custom-built hierarchical RAM-based file system. Unlike previous versions that operated as standard application shells, V18.0 implements low-level kernel entry protocols and simulated hardware abstractions. It provides a DOS-like environment with support for recursive directory navigation and basic file manipulation.

## Technical Specifications
* **Architecture:** Simulated Bare-Metal x86 Kernel.
* **Memory Management:** Fixed-pool node allocation for file system entities.
* **File System:** Linked-node hierarchical structure (RAM Disk).
* **I/O System:** Custom VGA driver implementation for color-coded console output.



## Core Functionality
The system operates through a specialized kernel entry point (`kernel_main`) and manages data through FsNode structures.

### File System Commands
* **DIR**: Lists files in the current directory.
* **DIR /S**: Executes a recursive search and display of all nested directories and files.
* **CD <path>**: Navigates the directory tree.
* **CD ..**: Returns to the parent directory node.
* **COPY <source> <destination>**: Performs a bitwise copy of file data and metadata to a new node.

### System Utilities
* **FNAF**: Initializes the Security Protocol sub-routine.
* **CALC**: Launches the internal arithmetic processor.
* **CLS**: Clears the VGA frame buffer.
* **BUDGET**: Accesses the internal financial tracking placeholder.

## Compilation and Installation
The source code is written in C++ with low-level assembly volatile blocks. It requires a compiler capable of handling freestanding environments.

### Requirements
* G++ or Clang++
* Linux, macOS, or Android (via Termux)
* C++11 standard or higher

### Build Instructions
```bash
g++ main.cpp -o sdos_v18
./sdos_v18
