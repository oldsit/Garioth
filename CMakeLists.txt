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

# Add ImGui source files
set(IMGUI_SOURCES
    ${PROJECT_SOURCE_DIR}/include/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/include/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/include/imgui/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/include/imgui/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/include/imgui/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp
    ${PROJECT_SOURCE_DIR}/include/imgui/backends/imgui_impl_opengl3.cpp
)

# Add STB Image header
set(STB_IMAGE_HEADER ${PROJECT_SOURCE_DIR}/include/stb_image.h)

# Executables
add_executable(garioth_server ${SERVER_SRC})
add_executable(garioth_client ${CLIENT_SRC})
add_executable(garioth_login ${LOGIN_SRC} ${GLAD_SRC} ${IMGUI_SOURCES})

# Include directories
target_include_directories(garioth_server PRIVATE 
    ${PROJECT_SOURCE_DIR}/src/common 
    ${PROJECT_SOURCE_DIR}/include 
    "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include"
    "C:/Program Files/OpenSSL-Win64/include"
    ${STB_IMAGE_HEADER}
)

target_include_directories(garioth_client PRIVATE 
    ${PROJECT_SOURCE_DIR}/src/common 
    ${PROJECT_SOURCE_DIR}/include 
    "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include"
    "C:/Program Files/OpenSSL-Win64/include"
    ${STB_IMAGE_HEADER}
)

target_include_directories(garioth_login PRIVATE 
    ${PROJECT_SOURCE_DIR}/src/common 
    ${PROJECT_SOURCE_DIR}/src/login 
    "C:/local/glad/include" 
    "C:/local/glfw-3.4.bin.WIN64/include"
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/imgui
    "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include"
    "C:/Program Files/OpenSSL-Win64/include"
    ${STB_IMAGE_HEADER}
)

# Link libraries for Windows
if (WIN32)
  target_link_libraries(garioth_server ws2_32 
      "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib"
  )

  target_link_libraries(garioth_client ws2_32 
      "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib"
  )

  target_link_libraries(garioth_login ws2_32 
      "C:/local/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib" 
      "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib"
      opengl32
  )
endif()

# Find OpenGL and link it with the login executable
find_package(OpenGL REQUIRED)
target_link_libraries(garioth_login ${OPENGL_gl_LIBRARY} 
    "C:/local/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib" 
    opengl32
)

# Add preprocessor definitions for Unicode
add_definitions(-DUNICODE=0 -D_UNICODE=0)

# --- Bundle assets (fonts, images, etc.) ---
# Copy assets to the build directory during the build process
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${PROJECT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory..."
)

add_dependencies(garioth_login copy_assets)

message(STATUS "Assets will be available in the build directory.")
