#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "logger.h"  // Include your logger header file
#include <limits>

#pragma comment(lib, "ws2_32.lib")

// Function to convert const char* to wchar_t
std::wstring StringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (size_needed == 0) {
        return L"";  // Handle error gracefully
    }
    std::wstring wstr(size_needed - 1, 0);  // Avoid trailing null character
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
    return wstr;
}

int main() {
    Logger logger("client_log.txt");  // Create an instance of Logger

    WSADATA wsa;
    SOCKET clientSocket = INVALID_SOCKET;
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
        logger.log("Socket creation failed with error: " + std::to_string(WSAGetLastError()));
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
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
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    logger.log("Address format validated successfully.");

    // Connect to the server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logger.log("Connection to server failed with error: " + std::to_string(WSAGetLastError()));
        std::cerr << "Connection to server failed.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    logger.log("Connected to server.");

    char buffer[256];  // Buffer to store received data
    char continueChoice;

    do {
        // Receive prompt from server
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);  // Reserve space for null terminator
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';  // Null-terminate the string
            std::cout << "Server says: " << buffer << std::endl;
            logger.log("Received prompt from server: " + std::string(buffer));

            // Ask the user for input based on the server's prompt
            std::cout << "Please enter your choice (1 for Register, 2 for Login): ";
            std::string choice;
            std::getline(std::cin, choice);

            // Send user's choice to the server
            int bytesSent = send(clientSocket, choice.c_str(), choice.length(), 0);
            if (bytesSent == SOCKET_ERROR) {
                logger.log("Failed to send choice.");  // Log using Logger instance
                std::cerr << "Failed to send choice.\n";
                break;
            }
            logger.log("Sent choice: " + choice);

            // Continue with further steps (registration/login) based on choice
            if (choice == "1") {
                // Registration process
                std::string username, email, password;
                std::cout << "Enter desired username: ";
                std::getline(std::cin, username);
                std::cout << "Enter email: ";
                std::getline(std::cin, email);
                std::cout << "Enter desired password: ";
                std::getline(std::cin, password);

                // Format and send registration details to the server
                std::string registrationDetails = username + "|" + email + "|" + password;
                send(clientSocket, registrationDetails.c_str(), registrationDetails.length(), 0);

                // Receive registration result from server
                bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0';
                    std::cout << "Server says: " << buffer << std::endl;
                    logger.log("Registration result: " + std::string(buffer));
                } else {
                    logger.log("Failed to receive registration result.");
                    std::cerr << "Failed to receive registration result.\n";
                }
            } else if (choice == "2") {
                // Login process
                std::string username, password;
                std::cout << "Enter username: ";
                std::getline(std::cin, username);
                std::cout << "Enter password: ";
                std::getline(std::cin, password);

                // Format and send login details to the server
                std::string loginDetails = username + "|" + password;
                send(clientSocket, loginDetails.c_str(), loginDetails.length(), 0);

                // Receive login result from server
                bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0';
                    std::cout << "Server says: " << buffer << std::endl;
                    logger.log("Login result: " + std::string(buffer));
                } else {
                    logger.log("Failed to receive login result.");
                    std::cerr << "Failed to receive login result.\n";
                }
            } else {
                std::cerr << "Invalid choice." << std::endl;
            }
        } else {
            logger.log("Failed to receive data from the server.");
            std::cerr << "Failed to receive data from the server.\n";
            break;
        }

        std::cout << "Do you want to continue? (y/n): ";
        std::cin >> continueChoice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } while (continueChoice == 'y' || continueChoice == 'Y');

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();
    logger.log("Client disconnected.");

    return 0;
}
