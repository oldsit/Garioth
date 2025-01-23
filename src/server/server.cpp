#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")

// Function to handle each client connection
void handleClient(SOCKET clientSocket) {
    wchar_t buffer[256];

    // Send a welcome message to the client
    const wchar_t* welcomeMessage = L"Welcome to the server!";
    int msgLen = wcslen(welcomeMessage) * sizeof(wchar_t); // Calculate length in bytes
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

    // Clean up
    closesocket(clientSocket);
}

// Function to initialize the server and accept client connections
void startServer() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);

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
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept client connection.\n";
            continue;
        }

        std::cout << "Client connected, spawning thread to handle communication.\n";

        // Create a new thread for each client connection
        std::thread clientThread(handleClient, clientSocket);
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
