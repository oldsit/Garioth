cmake_minimum_required(VERSION 3.10)

# Project name
project(Garioth)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define source files
set(SERVER_SRC src/server/server.cpp)
set(CLIENT_SRC src/client/client.cpp)

# Add server executable
add_executable(garioth_server ${SERVER_SRC})

# Add client executable
add_executable(garioth_client ${CLIENT_SRC})

# Add the common include directory to both client and server
target_include_directories(garioth_server PRIVATE ${PROJECT_SOURCE_DIR}/src/common "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include")
target_include_directories(garioth_client PRIVATE ${PROJECT_SOURCE_DIR}/src/common "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include")

# Include directory for load_env.h and OpenSSL
target_include_directories(garioth_server PRIVATE ${PROJECT_SOURCE_DIR}/include "C:/Program Files/OpenSSL-Win64/include")
target_include_directories(garioth_client PRIVATE ${PROJECT_SOURCE_DIR}/include "C:/Program Files/OpenSSL-Win64/include")

# Link necessary libraries (Winsock for Windows), MariaDB Connector/C++, and OpenSSL
if (WIN32)
    target_link_libraries(garioth_server ws2_32 "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib" "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib" "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib")
    target_link_libraries(garioth_client ws2_32 "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib" "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib" "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib")
endif()

# Add definitions
add_definitions(-DUNICODE=0 -D_UNICODE=0)
