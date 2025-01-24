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

std::mutex client_mutex;  // Mutex for thread synchronization

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

// Function to handle user registration
void registerUser(const std::string& username, const std::string& email, const std::string& password, sql::Connection* conn, Logger& logger) {
    std::string salt;
    std::string hash;

    try {
        salt = BcryptUtils::generateSalt();
        hash = BcryptUtils::generateHash(password, salt);
    } catch (const std::runtime_error& e) {
        logger.log(e.what());
        return;
    }

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("INSERT INTO users (username, email, password_hash, salt) VALUES (?, ?, ?, ?)"));
        pstmt->setString(1, username);
        pstmt->setString(2, email);
        pstmt->setString(3, hash);
        pstmt->setString(4, salt);
        pstmt->executeUpdate();

        logger.log("User " + username + " registered successfully.");
    } catch (sql::SQLException& e) {
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
    } catch (sql::SQLException& e) {
        logger.log("Error authenticating user: " + std::string(e.what()));
    }
    return false;
}

// Function to handle each client connection
void handleClient(SOCKET clientSocket, Logger& logger, sql::Connection* conn) {
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

    // Receive data from the client (as wide characters)
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived / sizeof(wchar_t)] = L'\0';  // Null-terminate the string
        std::wcout << L"Received message: " << buffer << std::endl;

        // Convert wide char buffer to a standard string
        std::wstring ws(buffer);
        std::string receivedMessage(ws.begin(), ws.end());

        // Parse the received message
        std::vector<std::string> fields = splitString(receivedMessage, '|');
        if (fields.size() == 3) {
            std::string username = fields[0];
            std::string email = fields[1];
            std::string password = fields[2];

            // Register user (example, adjust as needed)
            registerUser(username, email, password, conn, logger);

            // Authenticate user
            if (authenticateUser(username, password, conn, logger)) {
                send(clientSocket, "Authentication successful", 25, 0);
            } else {
                send(clientSocket, "Authentication failed", 21, 0);
            }
        } else {
            std::cerr << "Invalid message format.\n";
        }
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

    // Load environment variables from .env file
    logger.log("Loading environment variables...");
    loadEnvFile(".env");

    // Retrieve database credentials from environment variables
    char* db_host = nullptr;
    char* db_port = nullptr;
    char* db_name = nullptr;
    char* db_user = nullptr;
    char* db_password = nullptr;

    _dupenv_s(&db_host, nullptr, "DB_HOST");
    _dupenv_s(&db_port, nullptr, "DB_PORT");
    _dupenv_s(&db_name, nullptr, "DB_NAME");
    _dupenv_s(&db_user, nullptr, "DB_USER");
    _dupenv_s(&db_password, nullptr, "DB_PASSWORD");

    if (!db_host || !db_port || !db_name || !db_user || !db_password) {
        logger.log("Failed to retrieve environment variables.");
        return;
    }

    // Construct the connection string
    std::string db_url = "jdbc:mariadb://" + std::string(db_host) + ":" + std::string(db_port) + "/" + std::string(db_name);

    logger.log("Connecting to the database...");
    sql::Driver* driver = sql::mariadb::get_driver_instance();
    std::unique_ptr<sql::Connection> conn;
    try {
        conn = std::unique_ptr<sql::Connection>(driver->connect(db_url, db_user, db_password));
    } catch (sql::SQLException& e) {
        logger.log("Error connecting to the database: " + std::string(e.what()));
        free(db_host);
        free(db_port);
        free(db_name);
        free(db_user);
        free(db_password);
        return;
    }

    free(db_host);
    free(db_port);
    free(db_name);
    free(db_user);
    free(db_password);

    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    size_t clientAddrSize = sizeof(clientAddr);

    // Initialize Winsock
    logger.log("Initializing Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        logger.log("Failed to initialize Winsock.");
        return;
    }

    // Create the server socket
    logger.log("Creating server socket...");
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        logger.log("Failed to create server socket.");
        return;
    }

    // Bind the server socket to a specific port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000); // Port number
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    logger.log("Binding server socket...");
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logger.log("Failed to bind the socket.");
        return;
    }

    // Start listening for incoming connections
    logger.log("Listening on the server socket...");
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        logger.log("Failed to listen on the socket.");
        return;
    }

    logger.log("Server started and waiting for connections...");

    // Accept incoming client connections
    while (true) {
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, (int*)&clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            logger.log("Failed to accept client connection.");
            continue;
        }

        logger.log("Client connected.");

        // Create a new thread for each client connection
        std::thread clientThread(handleClient, clientSocket, std::ref(logger), conn.get());
        clientThread.detach();  // Detach the thread to run independently
    }

    // Cleanup
    logger.log("Cleaning up...");
    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    startServer();
    return 0;
}
