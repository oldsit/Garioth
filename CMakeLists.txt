cmake_minimum_required(VERSION 3.10)

project(Garioth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source files
set(SERVER_SRC src/server/server.cpp)
set(CLIENT_SRC src/client/client.cpp)
set(LOGIN_SRC src/login/login.cpp)

# Add glad source file to login executable
set(GLAD_SRC "C:/local/glad/src/glad.c")

# Executables
add_executable(garioth_server ${SERVER_SRC})
add_executable(garioth_client ${CLIENT_SRC})
add_executable(garioth_login ${LOGIN_SRC} ${GLAD_SRC})

# Include directories
target_include_directories(garioth_server PRIVATE ${PROJECT_SOURCE_DIR}/src/common 
                                                        "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include" 
                                                        ${PROJECT_SOURCE_DIR}/include 
                                                        "C:/Program Files/OpenSSL-Win64/include")
target_include_directories(garioth_client PRIVATE ${PROJECT_SOURCE_DIR}/src/common 
                                                        "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include" 
                                                        ${PROJECT_SOURCE_DIR}/include 
                                                        "C:/Program Files/OpenSSL-Win64/include")
target_include_directories(garioth_login PRIVATE ${PROJECT_SOURCE_DIR}/src/common 
                                                      ${PROJECT_SOURCE_DIR}/src/login 
                                                      "C:/local/glad/include" 
                                                      "C:/local/glfw-3.4.bin.WIN64/include") 

# Link libraries for Windows
if (WIN32)
  target_link_libraries(garioth_server ws2_32 
                          "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib" 
                          "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib" 
                          "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib")
  target_link_libraries(garioth_client ws2_32 
                          "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib" 
                          "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib" 
                          "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib")
endif()

# Find OpenGL and link it with the login executable
find_package(OpenGL REQUIRED)
target_link_libraries(garioth_login ${OPENGL_gl_LIBRARY} 
                                    "C:/local/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib" 
                                    opengl32)

# Add preprocessor definitions for Unicode
add_definitions(-DUNICODE=0 -D_UNICODE=0)
