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

        // Receive prompt from server
        wchar_t buffer[256];  // Buffer to store received data as wide characters
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived / sizeof(wchar_t)] = L'\0';  // Null-terminate the string
            std::wcout << L"Server says: " << buffer << std::endl;
            logger.log("Received prompt from server: " + std::string(buffer, buffer + bytesReceived));

            // Ask the user for input based on the server's prompt
            std::cout << "Please enter your choice: ";
            std::string choice;
            std::getline(std::cin, choice);

            // Send user's choice to the server
            int bytesSent = send(clientSocket, choice.c_str(), choice.length(), 0);
            if (bytesSent == SOCKET_ERROR) {
                logger.log("Failed to send choice.");  // Log using Logger instance
                std::cerr << "Failed to send choice.\n";
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
            logger.log("Sent choice: " + choice);

            // Continue with further steps (login/registration) based on choice
            if (choice == "1") {
                // Login process
                std::string username;
                std::cout << "Enter username: ";
                std::getline(std::cin, username);
                send(clientSocket, username.c_str(), username.length(), 0);

                std::string password;
                std::cout << "Enter password: ";
                std::getline(std::cin, password);
                send(clientSocket, password.c_str(), password.length(), 0);

                // Receive login result from server
                bytesReceived = recv(clientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
                if (bytesReceived > 0) {
                    buffer[bytesReceived / sizeof(wchar_t)] = L'\0';
                    std::wcout << L"Server says: " << buffer << std::endl;
                    logger.log("Login result: " + std::string(buffer, buffer + bytesReceived));
                } else {
                    std::cerr << "Failed to receive login result.\n";
                }
            } else if (choice == "2") {
                // Registration process
                std::string username;
                std::cout << "Enter desired username: ";
                std::getline(std::cin, username);

                std::string email;
                std::cout << "Enter email: ";
                std::getline(std::cin, email);

                std::string password;
                std::cout << "Enter desired password: ";
                std::getline(std::cin, password);

                // Format and send registration details to the server
                std::string registrationDetails = username + "|" + email + "|" + password;
                send(clientSocket, registrationDetails.c_str(), registrationDetails.length(), 0);

                // Receive registration result from server
                bytesReceived = recv(clientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
                if (bytesReceived > 0) {
                    buffer[bytesReceived / sizeof(wchar_t)] = L'\0';
                    std::wcout << L"Server says: " << buffer << std::endl;
                    logger.log("Registration result: " + std::string(buffer, buffer + bytesReceived));
                } else {
                    std::cerr << "Failed to receive registration result.\n";
                }
            }
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