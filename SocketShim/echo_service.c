#include <winsock2.h>
#include <stdio.h>
#include <process.h> // For _beginthreadex

#define NUM_THREADS 2

void echoThread(void* param) {
    int port = *((int*)param);

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return;
    }

    // Create a UDP socket
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == INVALID_SOCKET) {
        fprintf(stderr, "Error creating socket. Error code: %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    // Bind the UDP socket to the specified port
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    int enable = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR) {
        fprintf(stderr, "Error setting SO_REUSEADDR. Error code: %d\n", WSAGetLastError());
        closesocket(udpSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    if (bind(udpSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        fprintf(stderr, "Error binding socket. Error code: %d\n", WSAGetLastError());
        closesocket(udpSocket);
        WSACleanup();
        return;
    }

    printf("UDP Echo Server is running on port %d...\n", port);

    while (1) {
        char buffer[2056]; // Adjust the buffer size as needed
        struct sockaddr_in clientAddress;
        int clientAddressSize = sizeof(clientAddress);

        // Receive data using recvfrom
        int bytesReceived = recvfrom(udpSocket, buffer, sizeof(buffer), 0,
                                     (struct sockaddr*)&clientAddress, &clientAddressSize);

        if (bytesReceived == SOCKET_ERROR) {
            fprintf(stderr, "Error receiving data. Error code: %d\n", WSAGetLastError());
            break;
        }

        if (bytesReceived > 0) {
            printf("Received %d bytes from %s:%d\n", bytesReceived,
                   inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

            // Echo the received data back to the sender
            int bytesSent = sendto(udpSocket, buffer, bytesReceived, 0,
                                   (struct sockaddr*)&clientAddress, clientAddressSize);

            if (bytesSent == SOCKET_ERROR) {
                fprintf(stderr, "Error sending data. Error code: %d\n", WSAGetLastError());
                break;
            }

            printf("Echoed %d bytes back to %s:%d\n", bytesSent,
                   inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        }
    }

    // Cleanup
    closesocket(udpSocket);
    WSACleanup();
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return EXIT_FAILURE;
    }

    int ports[NUM_THREADS] = { 12352, 12354 };

    // Create threads
    HANDLE threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)echoThread, &ports[i], 0, NULL);
        if (threads[i] == NULL) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return EXIT_FAILURE;
        }
    }

    // Wait for threads to finish
    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    // Close thread handles
    for (int i = 0; i < NUM_THREADS; ++i) {
        CloseHandle(threads[i]);
    }

    // Cleanup
    WSACleanup();

    return 0;
}