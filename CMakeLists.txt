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



# Add the common include directory to both client and server after targets are created
target_include_directories(garioth_server PRIVATE ${PROJECT_SOURCE_DIR}/src/common)
target_include_directories(garioth_client PRIVATE ${PROJECT_SOURCE_DIR}/src/common)

# Add definitions
add_definitions(-DUNICODE=0 -D_UNICODE=0)
message(STATUS "Include directories for garioth_server: ${PROJECT_SOURCE_DIR}/src/common")
message(STATUS "Include directories for garioth_client: ${PROJECT_SOURCE_DIR}/src/common")

# Link necessary libraries (Winsock for Windows)
if (WIN32)
    target_link_libraries(garioth_server ws2_32)
    target_link_libraries(garioth_client ws2_32)
endif()
