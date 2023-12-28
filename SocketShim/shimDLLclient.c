/************************
 * Use this to compile:
 * 
cl.exe /nologo /Od /LD /EHsc /W4 /D_USRDLL /D_WINDLL shimDLLclient.c  /link ws2_32.lib  /OUT:SocketShimDLL.dll /DEF:RP1210.def

Use this to test:

simpleRP1210.exe shimDLL.dll 2


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Based on the guidance from https://beej.us/guide/bgnet/pdf/bgnet_usl_c_1.pdf
// we need to pull in the windows libraries as such:
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>  // for _getcwd


#include "shimDLLclient.h"

#define XTERNAL_DLL "DGDPA5MA.dll"

SOCKET send_server_socket, read_server_socket, message_socket;
struct sockaddr_in send_server_address, read_server_address, message_address;
int send_server_len = sizeof(send_server_address);
int read_server_len = sizeof(read_server_address);
int message_len = sizeof(message_address);

#define SendMessagePort 12352
#define ReadMessagePort 12354
#define OtherMessagePort 12353
#define MAX_BUFFER_SIZE 64

char messageBuffer[1480];

HMODULE dll_module;

/* Declare RP1210 functions to be loaded from the external DLL */
CLIENTCONNECT Xternal_RP1210_ClientConnect = NULL;
CLIENTDISCONNECT Xternal_RP1210_ClientDisconnect = NULL;
SENDMESSAGE Xternal_RP1210_SendMessage = NULL;
READMESSAGE Xternal_RP1210_ReadMessage = NULL;
SENDCOMMAND Xternal_RP1210_SendCommand = NULL;
READVERSION Xternal_RP1210_ReadVersion = NULL;
READDETAILEDVERSION Xternal_RP1210_ReadDetailedVersion = NULL;
GETHARDWARESTATUS Xternal_RP1210_GetHardwareStatus = NULL;
GETHARDWARESTATUSEX Xternal_RP1210_GetHardwareStatusEx = NULL;
GETERRORMESSAGE Xternal_RP1210_GetErrorMsg = NULL;
GETLASTERRORMESSAGE Xternal_RP1210_GetLastErrorMsg = NULL;
IOCTL Xternal_RP1210_Ioctl = NULL;

void HandleIniError(const char *section, const char *key, const char *filename) {
    fprintf(stderr, "Error reading [%s]%s from %s. Error code: %lu\n", section, key, filename, GetLastError());
}

void setupShimSocket(void){

    // Look for the ini file that has the IP address port and device driver.
    const char *filename = "shim_cfg.ini";  
    printf("\nReading Socket and VDA configuration from %s.\n", filename);
    
    //https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getprivateprofilestring
    //The name of the initialization file. 
    //If this parameter does not contain a full path to the file, 
    //the system searches for the file in the Windows directory.
    char currentDirectory[MAX_PATH];
    _getcwd(currentDirectory, sizeof(currentDirectory));
    printf("Current Directory: %s\n", currentDirectory);

    // Allocate enough space for the full path
    char fullPath[MAX_PATH];
    
    // Concatenate the current directory and file name
    snprintf(fullPath, sizeof(fullPath), "%s\\%s", currentDirectory, filename);

    // Now fullPath contains the full path to the file
    printf("Full Path for ini file: %s\n", fullPath);
     // Reading VDA string for the DLL

    printf("Loading VDA Device Driver DLL...\n");
    char xternal_rp1210dll_name[MAX_BUFFER_SIZE];
    if (GetPrivateProfileString("RP1210", "VDAdriver", XTERNAL_DLL, xternal_rp1210dll_name, MAX_BUFFER_SIZE, fullPath) == 0) {
        printf("There needs to be a valid RP1210 Vehicle Diagnostics Adapter installed on the system.\nPlease check %s for the right filename.\n", filename);
        HandleIniError("RP1210", "VDAdriver", filename);
        strncpy_s(xternal_rp1210dll_name,13,XTERNAL_DLL,13);
    } 
    dll_module = LoadLibrary(xternal_rp1210dll_name);
    printf("Done loading %s.\n", xternal_rp1210dll_name);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Loaded %s RP1210 Library.", xternal_rp1210dll_name);
    short bytesSent = send(message_socket, messageBuffer, strlen(messageBuffer), 0); 
    
    // Reading IP Address
    printf("Reading IP Address from the device file...");
    char ipBuffer[MAX_BUFFER_SIZE];
    // Set default value for IP address
    strncpy_s(ipBuffer,  MAX_BUFFER_SIZE, "127.0.0.1", 10);
    if (GetPrivateProfileString("Network", "IPAddress", "127.0.0.1", ipBuffer, MAX_BUFFER_SIZE, fullPath) == 0) {
        HandleIniError("Network", "IPAddress", filename);
        printf("Using Default IP Address: %s\n", ipBuffer);
    } else {
        printf("Found IP Address: %s\n", ipBuffer);
    }

    // Reading read_message_port
    printf("Reading Port from the device file...");
    int read_message_port = GetPrivateProfileInt("Network", "ReadMessagePort", ReadMessagePort, fullPath);
    if (read_message_port == 0 && GetLastError() != ERROR_SUCCESS) {
        HandleIniError("Network", "ReadMessagePort", filename);
        read_message_port = ReadMessagePort;
        printf("Using Default Port: %d\n", read_message_port);
    } else {
        printf("Using Read Message Port: %d\n", read_message_port);
    }
    
    // Reading send_message_port
    int send_message_port = GetPrivateProfileInt("Network", "SendMessagePort", SendMessagePort, fullPath);
    if (send_message_port == 0 && GetLastError() != ERROR_SUCCESS) {
        HandleIniError("Network", "SendMessagePort", filename);
        send_message_port = SendMessagePort;
        printf("Using Default Port: %d\n", send_message_port);
    } else {
        printf("Using Send Message Port: %d\n", send_message_port);
    }

    // Reading send_message_port
    int other_message_port = GetPrivateProfileInt("Network", "OtherMessagePort", OtherMessagePort, fullPath);
    if (other_message_port == 0 && GetLastError() != ERROR_SUCCESS) {
        HandleIniError("Network", "OtherMessagePort", filename);
        other_message_port = SendMessagePort;
        printf("Using Default Port: %d\n", send_message_port);
    } else {
        printf("Using Other Message Port: %d\n", send_message_port);
    }
    
   
    // Initialize Winsock
    printf("Initializing Winsock...");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }
    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2){
        fprintf(stderr,"Version 2.2 of Winsock is not available.\n");
        WSACleanup();
        exit(2);
    }
    // Create a socket
    if ((send_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        exit(3);
    }
     // Create a socket
    if ((read_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        exit(4);
    }
     // Create a socket
    if ((message_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        exit(4);
    }
    // Setting up send server address
    send_server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ipBuffer, &(send_server_address.sin_addr));
    send_server_address.sin_port = htons(send_message_port);

    // Setting up read server address
    read_server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ipBuffer, &(read_server_address.sin_addr));
    read_server_address.sin_port = htons(read_message_port);

    // Setting up other address
    message_address.sin_family = AF_INET;
    inet_pton(AF_INET, ipBuffer, &(message_address.sin_addr));
    message_address.sin_port = htons(other_message_port);
    
    // Set SO_REUSEADDR option
    int enable = 1;
    if (setsockopt(send_server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR) {
        fprintf(stderr, "Error setting SO_REUSEADDR. Error code: %d\n", WSAGetLastError());
        closesocket(send_server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    if (setsockopt(read_server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR) {
        fprintf(stderr, "Error setting SO_REUSEADDR. Error code: %d\n", WSAGetLastError());
        closesocket(read_server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    if (setsockopt(message_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR) {
        fprintf(stderr, "Error setting SO_REUSEADDR. Error code: %d\n", WSAGetLastError());
        closesocket(message_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("done.\n");

    // connect to the socket
    if (connect(send_server_socket, (struct sockaddr*)&send_server_address, sizeof(send_server_address)) == SOCKET_ERROR) {
        printf("Socket binding failed.\n");
        closesocket(send_server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else {
       printf("Successfully bound to send socket.\n"); 
    }

    // connect to the socket
    if (connect(read_server_socket, (struct sockaddr*)&read_server_address, sizeof(read_server_address)) == SOCKET_ERROR) {
        printf("Socket binding failed.\n");
        closesocket(read_server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else {
       printf("Successfully bound to read socket.\n"); 
    }

    // connect to the socket
    if (connect(message_socket, (struct sockaddr*)&message_address, sizeof(message_address)) == SOCKET_ERROR) {
        printf("Socket binding failed.\n");
        closesocket(message_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else {
       printf("Successfully bound to read socket.\n"); 
    }
    Xternal_RP1210_ClientConnect = GetProcAddress(dll_module, "RP1210_ClientConnect");
    Xternal_RP1210_ClientDisconnect = GetProcAddress(dll_module, "RP1210_ClientDisconnect");
    Xternal_RP1210_SendMessage = GetProcAddress(dll_module, "RP1210_SendMessage");
    Xternal_RP1210_ReadMessage = GetProcAddress(dll_module, "RP1210_ReadMessage");
    Xternal_RP1210_SendCommand = GetProcAddress(dll_module, "RP1210_SendCommand");       
    Xternal_RP1210_ReadVersion = GetProcAddress(dll_module, "RP1210_ReadVersion");
    Xternal_RP1210_ReadDetailedVersion = GetProcAddress(dll_module, "RP1210_ReadDetailedVersion");
    Xternal_RP1210_GetHardwareStatus = GetProcAddress(dll_module, "RP1210_GetHardwareStatus");
    Xternal_RP1210_GetHardwareStatusEx = GetProcAddress(dll_module, "RP1210_GetHardwareStatusEx");      
    Xternal_RP1210_GetErrorMsg = GetProcAddress(dll_module, "RP1210_GetErrorMsg");
    Xternal_RP1210_GetLastErrorMsg = GetProcAddress(dll_module, "RP1210_GetLastErrorMsg");
    Xternal_RP1210_Ioctl = GetProcAddress(dll_module, "RP1210_Ioctl");
    
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Welcome to the SocketShim System.\nLoaded all the external RP1210 function prototypes.");
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);   
}

short __declspec(dllexport) WINAPI RP1210_ClientConnect( 
                                        HWND   hwndClient,
                                        short  nDeviceID,
                                        char   *fpchProtocol,
                                        long   lTxBufferSize,
                                        long   lRxBufferSize,
                                        short  nIsAppPacketizingInMsgs){
    if (Xternal_RP1210_ClientConnect == NULL) setupShimSocket();
    short status = -ERR_DLL_NOT_INITIALIZED;
    status = Xternal_RP1210_ClientConnect(hwndClient, nDeviceID, 
            fpchProtocol, lTxBufferSize, lRxBufferSize, nIsAppPacketizingInMsgs);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Client Connnect. ClientID: %d, Device ID: %d, Protocol: %s",status,nDeviceID,fpchProtocol);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);     
    
    return(status);
}

short __declspec(dllexport) WINAPI RP1210_ClientDisconnect(short nClientID ){
    short status = -ERR_DLL_NOT_INITIALIZED;
    if (Xternal_RP1210_ClientDisconnect == NULL) setupShimSocket();
    status = Xternal_RP1210_ClientDisconnect(nClientID);
    printf("RP1210_ClientDisconnect Called and returned status %d...\n", status);
    // // Close sockets and clean up Winsock
    // closesocket(read_server_socket);
    // closesocket(send_server_socket);
    // WSACleanup();
    // printf("Finished closing sockets and WSACleanup");
    
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Client Disconnect. ClientID: %d, Return Value: %d",nClientID,status);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);     
    
    return(status);
}

short __declspec(dllexport) WINAPI RP1210_SendMessage( 
                                        short	nClientID,
                                        unsigned char *fpchClientMessage,
                                        short	nMessageSize,
                                        short   nNotifyStatusOnTx,
                                        short	nBlockOnSend ){
    
    if (Xternal_RP1210_SendMessage == NULL) setupShimSocket();
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    char rxbuffer[TXRXSIZE];
    /*
    Data coming in from the diagnositic application is contained in the buffer fpchClientMessage
    This data gets pushed out to a UDP socket and then waits for it to come back before it is sent to
    the vehicle diagnaostics adapter. There is an ineherent trust that the data on the port coming
    back is the right data. (It may have been manipulated, but is correctly formed.)
    
    From RP1210: The [RP1210_SendMessage] function will always return a value of 0 for successful 
    queuing of the message for transmission. In the event of an error in queuing the message for 
    transmission, an error code, corresponding to a value of greater than 127 is returned.
    */
    
    //Send the data received from the diagnostics software and send it to a the socket.
    short bytesSent = send(send_server_socket, fpchClientMessage, nMessageSize, 0);     
    if (bytesSent == SOCKET_ERROR) {
        fprintf(stderr, "Error sending data. Error code: %d\n", WSAGetLastError());
        closesocket(send_server_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }
    
    //Receive the data from the socket at send it onto the vehicle diagnostics adapter (VDA).
    short bytesRcvd = recvfrom(send_server_socket, rxbuffer, sizeof(rxbuffer), 0,
            (struct sockaddr*)&clientAddress, &clientAddressSize);
    
    if (bytesRcvd > sizeof(rxbuffer)){
        //return from Readmessage is messed up and likely too long. 
        //This may be from the system accumulating data in the background.
        return(-ERR_MESSAGE_TOO_LONG);
    }
    short status = -ERR_DLL_NOT_INITIALIZED; 
    if (bytesRcvd > 0){
        //Copy over the data from the socket to the array for the VDA.
        for (short i = 0; i< bytesRcvd; i++){
            fpchClientMessage[i] = rxbuffer[i];
        }
        nMessageSize = bytesRcvd;
        status = Xternal_RP1210_SendMessage(nClientID,
                                            fpchClientMessage,
                                            nMessageSize,
                                            nNotifyStatusOnTx,
                                            nBlockOnSend);
    }
    else {
        status = -ERR_TX_QUEUE_CORRUPT;
    }
    return(status);
}

short __declspec(dllexport) WINAPI RP1210_ReadMessage(
                                        short   nClientID,
                                        unsigned char *fpchAPIMessage,
                                        short	nBufferSize,
                                        short	nBlockOnRead ){
    if (Xternal_RP1210_ReadMessage == NULL) setupShimSocket();
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    char rxbuffer[TXRXSIZE];
    short status = -ERR_DLL_NOT_INITIALIZED; 
    status = Xternal_RP1210_ReadMessage(nClientID,
                                        fpchAPIMessage,
                                        nBufferSize,
                                        nBlockOnRead);
    /*
    Data coming in from the vehicle diagnostic adapter (VDA) is contained in the buffer fpchAPIMessage
    This data gets pushed out to a UDP socket and then waits for it to come back. There is an ineherent
    trust that the data on the port coming back is the right data. (It may have been manipulated, but is
    correctly formed.)
    */
    if (status > 0){ 
        //Send the data received from the external RP1210 adapter and send it to a the socket.
        int bytesSent = send(read_server_socket, fpchAPIMessage, status, 0);     
        if (bytesSent == SOCKET_ERROR) {
            fprintf(stderr, "Error sending data. Error code: %d\n", WSAGetLastError());
            closesocket(read_server_socket);
            WSACleanup();
            return EXIT_FAILURE;
        }
        
        //Recieve the data from the socket at send it onto the diagnositics program.
        short bytesRcvd = recvfrom(read_server_socket, rxbuffer, sizeof(rxbuffer), 0,
                (struct sockaddr*)&clientAddress, &clientAddressSize);
        
        if (bytesRcvd > sizeof(rxbuffer)){
            //return from Readmessage is messed up and likely too long. 
            //This may be from the system accumulating data in the background.
            return(-ERR_MESSAGE_TOO_LONG);
        }
        if (bytesRcvd > 0){
            //Copy over the data from the socket to the array for the VDA.
            for (short i = 0; i< bytesRcvd; i++){
                fpchAPIMessage[i] = rxbuffer[i];
            }
            return((short)bytesRcvd); // This is the status
        }
    }
    return(status);     
}

short __declspec(dllexport) WINAPI RP1210_SendCommand( 
                                        short   nCommandNumber,
                                        short 	nClientID,
                                        unsigned char *fpchClientCommand,
                                        short	nMessageSize ){
    if (Xternal_RP1210_SendCommand == NULL) setupShimSocket();
    short status = -ERR_DLL_NOT_INITIALIZED;
    status = Xternal_RP1210_SendCommand(nCommandNumber,
                                        nClientID,
                                        fpchClientCommand,
                                        nMessageSize);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Send Command number %d for Client ID %d. Contents: %X (%d bytes). Return Value: %d",nCommandNumber,nClientID,fpchClientCommand,nMessageSize,status);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);     
    
    return(status);                 
}

void __declspec(dllexport) WINAPI RP1210_ReadVersion(
                                        char *fpchDLLMajorVersion,
                                        char *fpchDLLMinorVersion,
                                        char *fpchAPIMajorVersion,
                                        char *fpchAPIMinorVersion
                                    ) {

    if (Xternal_RP1210_ReadVersion == NULL) setupShimSocket();
    Xternal_RP1210_ReadVersion( fpchDLLMajorVersion, 
                                fpchDLLMinorVersion, 
                                fpchAPIMajorVersion, 
                                fpchAPIMinorVersion);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Message: Read Version. DLL Major: %s. DLL Minor: %s, API Major: %s, API Minor: %s",
                                fpchDLLMajorVersion, 
                                fpchDLLMinorVersion, 
                                fpchAPIMajorVersion, 
                                fpchAPIMinorVersion);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);     
  
}

short __declspec(dllexport) WINAPI RP1210_ReadDetailedVersion(
                                        short nClientID,
                                        char* fpchAPIVersionInfo,
                                        char* fpchDLLVersionInfo,
                                        char* fpchFWVersionInfo) {
    if (Xternal_RP1210_ReadDetailedVersion == NULL) setupShimSocket();
    short status = -ERR_DLL_NOT_INITIALIZED;
    status = Xternal_RP1210_ReadDetailedVersion(nClientID, 
                                                fpchAPIVersionInfo, 
                                                fpchDLLVersionInfo, 
                                                fpchFWVersionInfo);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Message: Read Detailed Version. ClientID: %d, APIVersionInfo: %s, DLLVersionInfo: %s, FWVersionInfo: %s",
                                nClientID,
                                fpchAPIVersionInfo, 
                                fpchDLLVersionInfo, 
                                fpchFWVersionInfo);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);     
    return status;

}

short __declspec(dllexport) WINAPI RP1210_GetHardwareStatus(
                                        short nClientID,
                                        unsigned char *fpchClientInfo,
                                        short nInfoSize,
                                        short nBlockOnRequest) {

    if (Xternal_RP1210_GetHardwareStatus == NULL) setupShimSocket();
    short status = -ERR_DLL_NOT_INITIALIZED;
    status = Xternal_RP1210_GetHardwareStatus(nClientID, fpchClientInfo, nInfoSize, nBlockOnRequest);


    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Message: Get Hardware Status. ClientID: %d, HW Status: %X, status: %d",
                                nClientID,
                                fpchClientInfo, 
                                status);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);
    return status;
}

short __declspec(dllexport) WINAPI  RP1210_GetHardwareStatusEx(
                                        short   nClientID, 
                                        unsigned char *fpchClientInfo){
    if (Xternal_RP1210_GetHardwareStatusEx == NULL) setupShimSocket();
    short status = -ERR_DLL_NOT_INITIALIZED;
    status = Xternal_RP1210_GetHardwareStatusEx(nClientID, fpchClientInfo);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Message: Get Hardware Status Extended. ClientID: %d, HW Status: %s, status: %d",
                                nClientID,
                                fpchClientInfo, 
                                status);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);
    return status;
}

short __declspec(dllexport) WINAPI RP1210_GetErrorMsg(
                                        short errorCode,
                                        char *fpchDescription) {
    if (Xternal_RP1210_GetErrorMsg == NULL) setupShimSocket();
    short status = -ERR_DLL_NOT_INITIALIZED;
    status = Xternal_RP1210_GetErrorMsg(errorCode, fpchDescription);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Message: Get Error Message. Error Code: %d, Description: %s, status: %d",
                                errorCode,
                                fpchDescription, 
                                status);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);
    return status;
}

short __declspec(dllexport) WINAPI RP1210_GetLastErrorMsg(
                                        short   errorCode,
                                        int     *SubErrorCode,
                                        char    *fpchDescription,
                                        short   nClientID ) {
    if (Xternal_RP1210_GetLastErrorMsg == NULL) setupShimSocket();
    short status = -ERR_DLL_NOT_INITIALIZED;
    status = Xternal_RP1210_GetLastErrorMsg(errorCode, SubErrorCode, fpchDescription, nClientID);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Message: Get Error Message. Error Code: %d, Sub Error Code: %d, Description: %s, ClientID: %d, status: %d",
                                errorCode,
                                SubErrorCode,
                                fpchDescription,
                                nClientID, 
                                status);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);
    return status;
}

short __declspec(dllexport) WINAPI RP1210_Ioctl(
                                        short   nClientID,
                                        long    nIoctlID,
                                        void    *pInput,
                                        void    *pOutput ) {
    if (Xternal_RP1210_Ioctl == NULL) setupShimSocket();    
    short status = -ERR_DLL_NOT_INITIALIZED;
    status = Xternal_RP1210_Ioctl(nClientID, nIoctlID, pInput, pOutput);
    memset(messageBuffer,0,sizeof(messageBuffer));
    sprintf(messageBuffer,"Message: Ioctl. IoctlID: %d, ClientID: %d, status: %d",
                                nIoctlID,
                                nClientID, 
                                status);
    send(message_socket, messageBuffer, strlen(messageBuffer), 0);
    return status;
}