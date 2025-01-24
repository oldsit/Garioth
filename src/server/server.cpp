#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <string>
#include <mutex>
#include "logger.h"
#include <mariadb/conncpp.hpp>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4267)

std::mutex client_mutex;  // Mutex for thread synchronization

// Function to handle each client connection
void handleClient(SOCKET clientSocket, Logger& logger) {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    getpeername(clientSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));

    // Log client connection
    {
        std::lock_guard<std::mutex> lock(client_mutex);  // Locking the mutex for thread-safe logging
        logger.log("IP Address: " + std::string(clientIP) + " has connected.");
    }

    wchar_t buffer[256];
    
    // Send a welcome message to the client
    const wchar_t* welcomeMessage = L"Welcome to the Gariath!";
    int msgLen = wcslen(welcomeMessage) * sizeof(wchar_t);  // Calculate byte length for wide characters
    send(clientSocket, reinterpret_cast<const char*>(welcomeMessage), msgLen, 0);

    // Receive data from the client (as wide characters)
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived / sizeof(wchar_t)] = L'\0';  // Null-terminate the string
        std::wcout << L"Received message: " << buffer << std::endl;

        // Echo the message back to the client
        send(clientSocket, reinterpret_cast<const char*>(buffer), bytesReceived, 0);  // Send as wide char
    } else {
        std::cerr << "Failed to receive data from client.\n";
    }

    // Log client disconnection
    {
        std::lock_guard<std::mutex> lock(client_mutex);  // Locking the mutex for thread-safe logging
        logger.log("IP Address: " + std::string(clientIP) + " has disconnected.");
    }

    // Clean up
    closesocket(clientSocket);
}

// Function to initialize the server and accept client connections
void startServer() {
    Logger logger;  // Create a logger instance to log events

    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    size_t clientAddrSize = sizeof(clientAddr);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Failed to initialize Winsock.\n";
        return;
    }

    // Create the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create server socket.\n";
        return;
    }

    // Bind the server socket to a specific port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000); // Port number
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind the socket.\n";
        return;
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on the socket.\n";
        return;
    }

    std::cout << "Server started and waiting for connections...\n";

    // Accept incoming client connections
    while (true) {
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, (int*)&clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept client connection.\n";
            continue;
        }

        std::cout << "Client connected.\n";

        // Create a new thread for each client connection
        std::thread clientThread(handleClient, clientSocket, std::ref(logger));
        clientThread.detach();  // Detach the thread to run independently
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    startServer();
    return 0;
}
