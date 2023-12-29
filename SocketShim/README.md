# SocketRP1210Shim

## Overview
SocketRP1210Shim is a middleware software tool developed for cybersecurity research in vehicle diagnostics, focusing on heavy vehicles using RP1210 compliant vehicle diagnostic adapters (VDAs). It acts as a shim layer between diagnostics software and VDA's device driver, facilitating detailed analysis and manipulation of diagnostic communications.

## Implementation Details
- **Language & Environment**: The software is developed in C for Windows, tailored to the common environment for VDAs and diagnostics software in the heavy vehicle industry.
- **Components**:
  - **Client**: A Windows DLL shim that interfaces between the diagnostics software and the VDA DLL.
  - **Server**: Handles socket communications, processing, and potentially manipulating the data transferred between the client and the diagnostics software.
- **Functionality**: The tool intercepts and processes RP1210 function calls, allowing for logging, examination, and modification of data flowing between the diagnostics software and VDA.
- **Server Protocol**: Uses UDP for low-latency communication on localhost or local area networks.

## Installation
1. **Pre-Requisites**: Administrator privileges on a Windows machine are required. Existing VDA drivers must be installed.
2. **Setup**: Follow the detailed instructions provided for installing the SocketShim DLL and the accompanying .ini file.

## Usage
- **Configuration**: Instructions on configuring the SocketShim and server for various use cases.
- **Running the Software**: Step-by-step guide on how to run the software, including examples of common use cases.

## Testing and Verification
- **Privilege Requirements**: Building and deploying the SocketShim system requires administrator privileges on Windows.
- **Integration with Diagnostics Software**: Guidelines on how to integrate and use the SocketShim with existing diagnostics software.

## Contributing
Guidelines for contributing to the SocketRP1210Shim project.

## License
This project is released under the MIT Open Source License. Refer to the LICENSE file for more details.
