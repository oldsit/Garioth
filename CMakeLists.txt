cmake_minimum_required(VERSION 3.10)

project(Garioth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the CURL_INCLUDE_DIR and CURL_LIBRARIES manually
set(CURL_INCLUDE_DIR "C:/GoliathGames/Garioth/include/curl/include")
set(CURL_LIBRARIES "C:/GoliathGames/Garioth/include/curl/build/lib/Debug/libcurl.a")  # Adjust path if needed

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src/common  # Include common directory for JSONReader
    ${PROJECT_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIR}  # Explicitly set CURL_INCLUDE_DIR
)

# Find cURL package (use custom path if necessary)
find_package(CURL REQUIRED)

# Define paths for source files
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

# Manifest file path
set(MANIFEST_FILE ${CMAKE_SOURCE_DIR}/Goriath.manifest)

# Add common source files
set(COMMON_SRC
    ${PROJECT_SOURCE_DIR}/src/common/image_utils.cpp
    ${PROJECT_SOURCE_DIR}/src/common/audio_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/common/miniaudio_impl.cpp  # Add miniaudio implementation file
    ${PROJECT_SOURCE_DIR}/src/common/json_reader.cpp     # Add json_reader source file
    ${PROJECT_SOURCE_DIR}/src/common/http.cpp           # Add http source file
)

# Add STB Image header
set(STB_IMAGE_HEADER ${PROJECT_SOURCE_DIR}/include/stb_image.h)

# Add Miniaudio header
set(MINIAUDIO_HEADER ${PROJECT_SOURCE_DIR}/include/miniaudio.h)

# Executables
add_executable(garioth_server ${SERVER_SRC})
add_executable(garioth_client ${CLIENT_SRC})
add_executable(garioth_login ${LOGIN_SRC} ${GLAD_SRC} ${IMGUI_SOURCES} ${COMMON_SRC})

# Include directories for all targets
target_include_directories(garioth_server PRIVATE 
    ${PROJECT_SOURCE_DIR}/src/common 
    ${PROJECT_SOURCE_DIR}/include 
    "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include"
    "C:/Program Files/OpenSSL-Win64/include"
    ${CURL_INCLUDE_DIR}  # Explicitly set CURL_INCLUDE_DIR
)

target_include_directories(garioth_client PRIVATE 
    ${PROJECT_SOURCE_DIR}/src/common 
    ${PROJECT_SOURCE_DIR}/include 
    "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include"
    "C:/Program Files/OpenSSL-Win64/include"
    ${CURL_INCLUDE_DIR}  # Explicitly set CURL_INCLUDE_DIR
)

target_include_directories(garioth_login PRIVATE 
    ${PROJECT_SOURCE_DIR}/src/common
    "C:/local/glad/include" 
    "C:/local/glfw-3.4.bin.WIN64/include"
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/imgui
    "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/include"
    "C:/Program Files/OpenSSL-Win64/include"
    ${PROJECT_SOURCE_DIR}/include/miniaudio  # Miniaudio include directory
    ${CURL_INCLUDE_DIR}  # Explicitly set CURL_INCLUDE_DIR
)

# Link directories for libraries
link_directories(
    "C:/GoliathGames/Garioth/include/curl/build/lib/Debug"  # Directory containing libcurl.a
)

# Link libraries for Windows
if (WIN32)
  target_link_libraries(garioth_server ws2_32 
      "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib"
      ${CURL_LIBRARIES}  # Add this line for cURL
  )

  target_link_libraries(garioth_client ws2_32 
      "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib"
      ${CURL_LIBRARIES}  # Add this line for cURL
  )

  # Link the manifest file for the login executable
  set_target_properties(garioth_login PROPERTIES 
      LINK_FLAGS "/MANIFEST /MANIFESTINPUT:${MANIFEST_FILE}")

  target_link_libraries(garioth_login 
      ws2_32 
      "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit/mariadbcpp.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib"
      "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libcrypto.lib"
      ${CURL_LIBRARIES}  # Add this line for cURL
  )
endif()

# Find OpenGL and link it with the login executable
find_package(OpenGL REQUIRED)
target_include_directories(garioth_login PRIVATE ${OPENGL_INCLUDE_DIR})
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
