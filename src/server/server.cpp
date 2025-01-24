#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <string>
#include <mutex>
#include <sstream>
#include <vector>
#include "logger.h"
#include <mariadb/conncpp.hpp>
#include "loadenv.h"
#include "bcrypt_utils.h"

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4267)

std::mutex client_mutex;

// Function to split a string by a delimiter
std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to register a user
void registerUser(const std::string& username, const std::string& email, const std::string& password, sql::Connection* conn, Logger& logger) {
    try {
        std::string salt = BcryptUtils::generateSalt();
        std::string hash = BcryptUtils::generateHash(password, salt);

        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("INSERT INTO users (username, email, password_hash, salt) VALUES (?, ?, ?, ?)"));
        pstmt->setString(1, username);
        pstmt->setString(2, email);
        pstmt->setString(3, hash);
        pstmt->setString(4, salt);
        pstmt->executeUpdate();

        logger.log("User " + username + " registered successfully.");
    } catch (const std::exception& e) {
        logger.log("Error registering user: " + std::string(e.what()));
    }
}

// Function to authenticate user
bool authenticateUser(const std::string& username, const std::string& password, sql::Connection* conn, Logger& logger) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("SELECT password_hash, salt FROM users WHERE username = ?"));
        pstmt->setString(1, username);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            std::string stored_hash = res->getString("password_hash");
            std::string salt = res->getString("salt");
            return BcryptUtils::validatePassword(password, salt, stored_hash);
        }
    } catch (const std::exception& e) {
        logger.log("Error authenticating user: " + std::string(e.what()));
    }
    return false;
}

// Function to handle client connections
void handleClient(SOCKET clientSocket, Logger& logger, std::string db_url, std::string db_user, std::string db_password) {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    getpeername(clientSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));

    logger.log("IP Address: " + std::string(clientIP) + " has connected.");

    sql::Driver* driver = sql::mariadb::get_driver_instance();
    std::unique_ptr<sql::Connection> conn(driver->connect(db_url, db_user, db_password));

    std::string initialMessage = "Welcome! Please choose an option:\n1. Register\n2. Login\n";
    send(clientSocket, initialMessage.c_str(), initialMessage.length(), 0);

    char buffer[256];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        logger.log("Client disconnected or error occurred.");
        closesocket(clientSocket);
        return;
    }

    buffer[bytesReceived] = '\0';
    std::string choice = buffer;

    if (choice == "1") {
        std::string registerMessage = "Enter username, email, password separated by '|':\n";
        send(clientSocket, registerMessage.c_str(), registerMessage.length(), 0);

        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::vector<std::string> fields = splitString(buffer, '|');
            if (fields.size() == 3) {
                registerUser(fields[0], fields[1], fields[2], conn.get(), logger);
                send(clientSocket, "Registration successful!\n", 26, 0);
            } else {
                send(clientSocket, "Invalid registration format.\n", 29, 0);
            }
        }
    } else if (choice == "2") {
        std::string loginMessage = "Enter username and password separated by '|':\n";
        send(clientSocket, loginMessage.c_str(), loginMessage.length(), 0);

        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::vector<std::string> fields = splitString(buffer, '|');
            if (fields.size() == 2 && authenticateUser(fields[0], fields[1], conn.get(), logger)) {
                send(clientSocket, "Authentication successful!\n", 28, 0);
            } else {
                send(clientSocket, "Authentication failed.\n", 24, 0);
            }
        }
    } else {
        send(clientSocket, "Invalid choice.\n", 16, 0);
    }

    logger.log("IP Address: " + std::string(clientIP) + " has disconnected.");
    closesocket(clientSocket);
}

void startServer() {
    Logger logger;
    logger.log("Loading environment variables...");
    loadEnvFile("C:/GoliathGames/Garioth/.env");

    char* db_host = nullptr, * db_port = nullptr, * db_name = nullptr, * db_user = nullptr, * db_password = nullptr;
    _dupenv_s(&db_host, nullptr, "DB_HOST");
    _dupenv_s(&db_port, nullptr, "DB_PORT");
    _dupenv_s(&db_name, nullptr, "DB_NAME");
    _dupenv_s(&db_user, nullptr, "DB_USER");
    _dupenv_s(&db_password, nullptr, "DB_PASSWORD");

    std::string db_url;

    if (!db_host || !db_port || !db_name || !db_user || !db_password) {
        logger.log("Failed to retrieve environment variables.");
        goto cleanup;
    }

    // Initialize db_url after ensuring variables are valid
    db_url = "jdbc:mariadb://" + std::string(db_host) + ":" + std::string(db_port) + "/" + std::string(db_name);


    free(db_host);
    free(db_port);
    free(db_name);
    free(db_user);
    free(db_password);

    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr;
    size_t clientAddrSize = sizeof(sockaddr_in);

    logger.log("Initializing Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) return;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) return;

    if (listen(serverSocket, 10) == SOCKET_ERROR) return;

    logger.log("Server started. Waiting for connections...");

    while (true) {
        clientSocket = accept(serverSocket, (sockaddr*)&serverAddr, (int*)&clientAddrSize);
        if (clientSocket == INVALID_SOCKET) continue;

        logger.log("Client connected.");
        std::thread clientThread(handleClient, clientSocket, std::ref(logger), db_url, db_user, db_password);
        clientThread.detach();
    }

cleanup:
    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    startServer();
    return 0;
}
