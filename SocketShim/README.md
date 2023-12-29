# SocketShim

## Overview
SocketRP1210Shim is a middleware software tool developed for vehicle diagnostics cybersecurity research, particularly for heavy vehicles using RP1210 compliant vehicle diagnostic adapters (VDAs). It acts as a shim layer between diagnostic software and VDA's device driver, enabling analysis and manipulation of diagnostic communications.

### Key Features and Functions:
1. **Interception and Redirection**: SocketShim operates by intercepting the communication between the vehicle's diagnostic software and the VDA's device driver.

2. **Shim Layer Mechanism**: The core functionality of SocketShim is encapsulated in its ability to act as a 'shim' or an intermediary layer.

3. **Diagnostic Communication Analysis**: SocketShim provides a robust platform for researchers to conduct in-depth analysis of diagnostic communications.

4. **Data Manipulation Capabilities**: Beyond mere analysis, SocketShim empowers users to modify the diagnostic data in transit. This functionality is particularly beneficial for simulating various cybersecurity scenarios, including attack patterns, data integrity checks, and the development of defensive strategies against potential cyber threats.

6. **Application in Cybersecurity Research**: The primary application of SocketShim lies in the domain of cybersecurity research. By facilitating detailed analysis and controlled manipulation of diagnostic data, it serves as a valuable tool for researchers focusing on vehicular cybersecurity, particularly in understanding and mitigating risks associated with VDA communications.

## Components

### 1. **ShimDLL Client (ShimDLL Client.c)**
- **Overview**: The ShimDLL Client is a crucial component written in C. It functions as the intermediary between the vehicle diagnostic software and the VDA's device driver. 
- **Key Functions**:
  - **Dynamic Link Library Interactions**: It dynamically loads the VDA's DLL and utilizes its exported functions for diagnostic communications.
  - **Network Communication**: Uses Windows sockets to establish UDP connections for transmitting and receiving diagnostic data.
  - **Data Handling**: Intercepts diagnostic messages and commands from the diagnostic software, forwards them to the SocketRp1210Server, and then sends the potentially modified responses back to the diagnostic software.
  - **Function Prototypes**: Implements prototypes like `RP1210_ClientConnect`, `RP1210_SendMessage`, `RP1210_ReadMessage`, etc., which mirror those in the VDA's DLL but with added functionality for interception and redirection.
- **Technical Implementation**:
  - The client sets up socket connections based on configuration read from `shim_cfg.ini`, handling different types of messages (send, read, other) on separate ports.
  - It wraps around the VDA's DLL functions, captures the data passed to these functions, and then communicates with the SocketRp1210Server for further processing.

### 2. **Echo Service (echo_service.c)**
- **Overview**: Echo Service is a multi-threaded UDP echo server designed to assist in testing and debugging the ShimDLL Client.
- **Key Functions**:
  - **UDP Socket Management**: Creates and manages multiple UDP sockets for echoing data.
  - **Multi-threading**: Leverages multi-threading capabilities to handle different ports simultaneously, enhancing its capacity to process concurrent diagnostic sessions.
  - **Data Echoing**: Receives diagnostic data from the ShimDLL Client and immediately echoes it back, allowing for verification of data integrity and transmission.
- **Technical Implementation**:
  - Utilizes the Winsock library for network communication, adhering to Windows networking standards.
  - Implements thread functions like `echoThread` to handle data on specified ports, showcasing an effective use of concurrency in network communication.

### 3. **SocketRp1210Server (SocketRp1210Server.py)**
- **Overview**: The SocketRp1210Server is a Python-based server that handles socket communications from the ShimDLL Client, offering the capability to manipulate diagnostic data in transit.
- **Key Functions**:
  - **Socket Communication**: Establishes UDP connections to receive data from the ShimDLL Client and send back responses.
  - **Data Manipulation**: Includes the `change_data` function to modify diagnostic data based on certain criteria, such as altering VIN or other specific data packets.
  - **Logging**: Implements advanced logging mechanisms to keep track of all data transmissions, aiding in data analysis and research.
- **Technical Implementation**:
  - Uses Python's socket library for network communication, ensuring cross-platform compatibility and ease of deployment.
  - Incorporates threading to handle multiple types of messages concurrently, thereby enhancing its responsiveness and capacity to handle complex diagnostic scenarios.


## Installation

### 1. **Requirements**
- **Operating System**: Windows OS with administrator privileges. The software components are designed to integrate with Windows-specific networking and DLL management features.
- **VDA Drivers**: Ensure that vehicle diagnostic adapter (VDA) drivers compatible with the RP1210 standard are pre-installed. These drivers are crucial for the ShimDLL Client to interact with the vehicle's diagnostics systems.

### 2. **Setup**
- **Compiling the Components**:
  - **ShimDLL Client & Echo Service**:
    - Use a C compiler like Microsoft Visual Studio C++ (MSVC) or GNU Compiler Collection (GCC) to compile the `ShimDLL Client.c` and `echo_service.c` files.
    - The resulting binaries should be `ShimDLLClient.dll` and `echo_service.exe` respectively.
  - **Python Environment**:
    - Ensure that Python (version 3.x or later) is installed on your system. Python is required to run the `SocketRp1210Server.py` script.
    - You can download Python from the official [Python website](https://www.python.org/downloads/).

- **Configuration**:
  - **shim_cfg.ini File**:
    - Locate the `shim_cfg.ini` file in the project directory. This file contains essential configuration settings for the ShimDLL Client.
    - Edit the file to specify the correct paths for the VDA DLL and network settings. 
    - Key parameters to configure include:
      - `VDAdriver`: The filename of the VDA DLL to be used.
      - `IPAddress`, `ReadMessagePort`, `SendMessagePort`, `OtherMessagePort`: Network settings for socket communication between the ShimDLL Client and the SocketRp1210Server.

- **Running the Components**:
  - **Echo Service**:
    - Run `echo_service.exe` to start the echo server. It will listen on the specified ports and echo received data, facilitating testing and debugging.
  - **SocketRp1210Server**:
    - Run `SocketRp1210Server.py` using Python. This starts the server which will handle socket communications from the ShimDLL Client and manipulate data as configured.
  - **ShimDLL Client**:
    - Ensure that the compiled `ShimDLLClient.dll` is located in a directory accessible by the diagnostic application that will use it.
    - Configure the diagnostic application to load `ShimDLLClient.dll` as its RP1210 driver.

### 3. **Verification**
- After installation, verify that each component is functioning correctly:
  - Check that the Echo Service echoes data correctly on the designated ports.
  - Ensure that the SocketRp1210Server logs data and manipulates it as expected.
  - Confirm that the ShimDLL Client successfully intercepts and redirects diagnostic commands and data between the diagnostic software and VDA.


## Usage

### A. SocketShim Client

The SocketShim Client, written in C, is designed to interface with the VDA's DLL and diagnostic software running on Windows. It functions by intercepting and redirecting RP1210 compliant commands.

1. **Initializing the Client**:
   - The client begins by loading the external VDA DLL specified in the `shim_cfg.ini` file.
   - It uses Windows dynamic linking (via `LoadLibrary` and `GetProcAddress`) to load and call functions from the VDA DLL.
   - Example functions include `RP1210_ClientConnect`, `RP1210_SendMessage`, and `RP1210_ReadMessage`.

2. **Handling RP1210 Commands**:
   - Each RP1210 function in the client acts as a pass-through, calling the corresponding function in the VDA DLL.
   - The `RP1210_ClientConnect` function, for example, initializes a connection with the VDA, passing parameters like `hwndClient`, `nDeviceID`, `fpchProtocol`, etc.
   - For `RP1210_ReadMessage`, the client receives data from the VDA DLL, sends it to the Socket Server via UDP, waits for a response, and then passes the (potentially modified) data back to the diagnostic application.

3. **Data Manipulation and Transmission**:
   - Data received from the VDA DLL can be sent to the Socket Server for logging or manipulation.
   - The client uses different UDP ports for sending and receiving messages, as configured in the `shim_cfg.ini` file.

### B. Socket Server

The Socket Server, written in Python, handles socket communications from the client. It runs on localhost or a local network, using UDP for low-latency message transmission.

1. **Echo Server Functionality**:
   - The server acts as an echo server, reflecting the network traffic back to the client.
   - It records the data in an ASCII representation of the bytes using Python's logging engine.
   - The `handle_RP1210ReadMessage` function demonstrates this by receiving data, logging it, and sending it back to the client.

2. **Custom Data Manipulation**:
   - The server has the ability to modify the data in transit.
   - Data passed to the `change_data` function can be customized based on specific criteria. This function is critical for simulating various diagnostic scenarios or testing data integrity.

3. **Configurable Ports and Logging**:
   - Ports for different message types (e.g., read and send message ports) are configurable in the `shim_cfg.ini` file.
   - All data transactions are logged, providing a detailed record of communications for analysis and research.

### Running the System

1. **Start the Socket Server**:
   - Run `SocketRp1210Server.py` to initiate the server.
   - Ensure it's listening on the correct ports as per the configuration.

2. **Operate the ShimDLL Client**:
   - With the diagnostic application running, the ShimDLL Client will intercept and process RP1210 commands.
   - Monitor the client's activity, ensuring it successfully captures and redirects data between the diagnostic software and the VDA.

3. **Analyze and Manipulate Data**:
   - Utilize the server's logging and `change_data` function to analyze and manipulate diagnostic data.
   - Experiment with different data scenarios to test the system's response and robustness.
## Testing and Verification

### Building and Deployment
1. **Administrator Privileges**: Ensure you have administrator rights on the Windows-based computer for installation and modification of system files.
2. **Installing the SocketShim System**:
   - Place the `SOCKSHIM.dll` (the compiled ShimDLL Client) and the `SOCKSHIM.ini` configuration file in the appropriate directory, typically `C:\Windows\SysWOW64`.
   - Update the `RP121032.ini` file in the `C:\Windows` directory to include `SOCKSHIM` in the `[RP1210Support]` section, allowing RP1210 compliant applications to recognize and use the ShimDLL Client.

### Testing Data Manipulation
1. **Manipulating Vehicle Identification Number (VIN)**:
   - Utilize the `change_data` function in the Socket Server to alter the VIN being broadcast.
   - For example, identify J1939 messages with a specific parameter group number (e.g., `0x00FEEC` for VIN) and replace the original VIN with a custom string (e.g., `b'TotallyFakeVIN'`).
   - Use an RP1210-compliant application like DG Diagnostics to query the VIN from a control unit. Compare the displayed VIN in the application with the manipulated VIN to verify successful data alteration.

2. **Digital Forensics Use Case**:
   - Test the system's ability to prevent manipulation of critical forensic data, such as preventing the reset of the ECU's real-time clock.
   - Intercept network traffic using the SocketShim that contains commands for resetting the clock.
   - Verify that these messages are either dropped or altered to maintain the integrity of time-sensitive event records (e.g., hard braking events).

## License
This project is released under the MIT Open Source License. Refer to the LICENSE file for more details.
