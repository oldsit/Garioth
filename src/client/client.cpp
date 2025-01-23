#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// Function to convert const char* to wchar_t
std::wstring StringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
    return wstr;
}

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    // Initialize winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Failed to initialize Winsock.\n";
        return 1;
    }

    // Create TCP Socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    // Define server address to connect to
    serverAddr.sin_family = AF_INET; // IPV4
    serverAddr.sin_port = htons(54000); // PORT

    // Convert "127.0.0.1" to wchar_t (wide string) for InetPtonW
    std::wstring wAddress = StringToWString("127.0.0.1");

    // Use InetPtonW for Windows wide string address conversion
    if (InetPtonW(AF_INET, wAddress.c_str(), &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid address format.\n";
        return 1;
    }

    // Connect to the server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) {
        char buffer[256];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the string
            std::cout << "Server says: " << buffer << std::endl;
        }
        else {
            std::cerr << "Failed to receive data from the server.\n";
        }
    } else {
        std::cerr << "Connection to server failed.\n";
    }

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
