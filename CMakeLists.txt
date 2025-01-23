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
target_include_directories(garioth_server PRIVATE include)

# Add client executable
add_executable(garioth_client ${CLIENT_SRC})
target_include_directories(garioth_client PRIVATE include)

# Link necessary libraries (Winsock for Windows)
if (WIN32)
    target_link_libraries(garioth_server ws2_32)
    target_link_libraries(garioth_client ws2_32)
endif()
