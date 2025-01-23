#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "logger.h"  // Include your logger header file

#pragma comment(lib, "ws2_32.lib")

// Function to convert const char* to wchar_t
std::wstring StringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
    return wstr;
}

int main() {
    Logger logger("client_log.txt");  // Create an instance of Logger

    WSADATA wsa;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    // Initialize winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        logger.log("Failed to initialize Winsock.");  // Log using Logger instance
        std::cerr << "Failed to initialize Winsock.\n";
        return 1;
    }
    logger.log("Winsock initialized successfully.");

    // Create TCP Socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        logger.log("Socket creation failed.");  // Log using Logger instance
        std::cerr << "Socket creation failed.\n";
        return 1;
    }
    logger.log("Socket created successfully.");

    // Define server address to connect to
    serverAddr.sin_family = AF_INET; // IPV4
    serverAddr.sin_port = htons(54000); // PORT

    // Convert "127.0.0.1" to wchar_t (wide string) for InetPtonW
    std::wstring wAddress = StringToWString("127.0.0.1");

    // Use InetPtonW for Windows wide string address conversion
    if (InetPtonW(AF_INET, wAddress.c_str(), &serverAddr.sin_addr) != 1) {
        logger.log("Invalid address format.");  // Log using Logger instance
        std::cerr << "Invalid address format.\n";
        return 1;
    }
    logger.log("Address format validated successfully.");

    // Connect to the server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) {
        logger.log("Connected to server.");  // Log using Logger instance
        std::cout << "Connected to server.\n";

        // Send data to the server (ensure it's wide characters)
        const wchar_t* msg = L"Hello, Server! This is the client sending a message in wchar_t.";
        int msgLen = wcslen(msg) * sizeof(wchar_t);  // Calculate byte length for wide characters
        int bytesSent = send(clientSocket, reinterpret_cast<const char*>(msg), msgLen, 0);
        if (bytesSent == SOCKET_ERROR) {
            logger.log("Failed to send data.");  // Log using Logger instance
            std::cerr << "Failed to send data.\n";
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        logger.log("Sent " + std::to_string(bytesSent) + " bytes to the server.");  // Log using Logger instance
        std::wcout << L"Sent " << bytesSent << L" bytes to the server.\n";

        // Receive response from the server
        wchar_t buffer[256];  // Buffer to store received data as wide characters
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived / sizeof(wchar_t)] = L'\0';  // Null-terminate the string

            // Convert wideBuffer to std::wstring for logging
            std::wstring receivedData(buffer);
            std::string logMessage = "Received data from server: " + std::string(receivedData.begin(), receivedData.end());

            logger.log(logMessage);  // Log the concatenated message
            std::wcout << L"Server says: " << buffer << std::endl;
        } else {
            logger.log("Failed to receive data from the server.");  // Log using Logger instance
            std::cerr << "Failed to receive data from the server.\n";
        }
    } else {
        logger.log("Connection to server failed.");  // Log using Logger instance
        std::cerr << "Connection to server failed.\n";
    }

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();
    logger.log("Client disconnected.");  // Log using Logger instance

    return 0;
}
