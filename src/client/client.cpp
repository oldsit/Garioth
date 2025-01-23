#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib");

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    // Initialize winsock
    if (WSAStartup(MAKEWORD(2, 2), & wsa) != 0) {
        std::cerr << "Failed to initailize Winsock.\n";
    }

    //Create TCP Socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    } 

    // Define server address to connect to. 
    serverAddr.sin_family = AF_INET; //IPV4
    serverAddr.sin_port = htons(54000); // PORT
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) {
        char buffer[256];
        recv(clientSocket, buffer, sizeof(buffer), 0);
        std::cout << "Server says: " << buffer << std::endl;
    } else {
        std::cerr << "Connection to server failed.\n";
    }

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}