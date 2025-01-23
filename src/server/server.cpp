#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib") // Link Winsock Library

int main() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(sockaddr_in);
    
    // Check for winsock errors
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Failed to initalize winsock.\n";
    }

    //Create TCP Socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    // Configure server address
    serverAddr.sin_family = AF_INET; // IPV4
    serverAddr.sin_port = htons(54000); // ON PORT 54000
    serverAddr.sin_addr.s_addr = INADDR_ANY; // USE ANY AVAILABLE NETWORK INTERFACE

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        return 1;
    }

    // Listen for connections
    listen(serverSocket, SOMAXCONN);
    std::cout << "Waiting for connections on port 54000...\n";

    // Accept incoming client connection reqs
    clientSocket = accept(serverSocket, (sockaddr*)& clientAddr, &addrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Client connection failed.\n";
        return 1;
    }

    std::cout << "Client connected.\n";

    // Send message to client
    const char* message = "Welcome to Garioth server";
    send(clientSocket, message, strlen(message), 0);

    // Cleanup sockets
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup;

}