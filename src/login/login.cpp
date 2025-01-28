#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
// Include ImGui headers
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <sstream>
#include <algorithm>
#include <windows.h>
#include <image_utils.h>
// #include "common/directory_utils.h"
#include <audio_manager.h>
#include <json_reader.h>
#include <http.h>

// Persistent settings variables
bool isDirectoryCreated = false;
static bool fullscreen = false;
static int currentResolution = 0;
static std::vector<std::string> resolutions;
static std::vector<const char*> resolutionCStrs;
static bool resolutionsLoaded = false;
static int currentResolutionIndex = 0;
static std::string resolution = "1920x1080"; // Default resolution
std::string path = "C:/Program Files/Goliath Games/Goriath/Screenshots/";


GLFWcursor* customCursor = nullptr; // Define customCursor globally

std::tuple<GLFWcursor*, int, int> CreateCustomCursor(const char* imagePath, int hotspotX, int hotspotY) {
    // Load the image using stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(imagePath, &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load cursor image from: " << imagePath << std::endl;
        return {nullptr, 0, 0}; // Return a null cursor and 0 dimensions
    }
    
    std::cout << "Image loaded: " << imagePath << " (Width: " << width << ", Height: " << height << ")" << std::endl;
    
    // Create the custom cursor
    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = data;
    
    // Create the custom cursor with the hotspot passed
    GLFWcursor* cursor = glfwCreateCursor(&image, hotspotX, hotspotY);
    
    // Free the image data after creating the cursor
    stbi_image_free(data);
    
    return {cursor, width, height}; // Return cursor and image dimensions
}

GLFWcursor* create_custom_cursor(const char* image_path) {
    int width, height, channels;
    unsigned char* data = stbi_load(image_path, &width, &height, &channels, 4);
    
    if (!data) {
        std::cerr << "Failed to load cursor image!" << std::endl;
        return nullptr;
    }

    GLFWimage cursorImage = {width, height, data};
    GLFWcursor* cursor = glfwCreateCursor(&cursorImage, 0, 0);
    stbi_image_free(data);

    return cursor;
}

GLFWcursor* ResizeCursor(GLFWcursor* originalCursor, const char* imagePath, int newWidth, int newHeight, int hotspotX, int hotspotY) {
    // Load the original image using stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(imagePath, &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load cursor image from: " << imagePath << std::endl;
        return nullptr; // Return a null cursor if loading failed
    }
    
    std::cout << "Image loaded: " << imagePath << " (Width: " << width << ", Height: " << height << ")" << std::endl;

    // Create a new buffer to hold the resized image data
    unsigned char* resizedData = new unsigned char[newWidth * newHeight * channels];
    
    // Resize the image to the new dimensions
    stbir_resize_uint8(data, width, height, 0, resizedData, newWidth, newHeight, 0, channels);

    // Create a GLFW image with the resized data
    GLFWimage image;
    image.width = newWidth;
    image.height = newHeight;
    image.pixels = resizedData;

    // Create the new cursor with the resized image
    GLFWcursor* newCursor = glfwCreateCursor(&image, hotspotX, hotspotY);
    
    // Free the original image data after resizing
    stbi_image_free(data);
    delete[] resizedData;  // Don't forget to free the resized data

    // Destroy the old cursor (if needed, handle the original cursor clean-up in your program)
    if (originalCursor) {
        glfwDestroyCursor(originalCursor);
    }
    
    // Return the new cursor
    return newCursor;
}


void GetAvailableResolutions(std::vector<std::string>& resolutions) {
    // Get prim monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (primaryMonitor) {
        // Get list of vid modes
        int count;
        const GLFWvidmode* modes = glfwGetVideoModes(primaryMonitor, &count);

        // Store in vect
        for (int i = 0; i <count; ++i) {
            resolutions.push_back(std::to_string(modes[i].width) + "x" + std::to_string(modes[i].height));
        }

    }
}

// Function to save settings to the registry
void SaveSettingsToRegistry(bool fullscreen, const std::string& resolution) {
    HKEY hKey;
    const char* regPath = "Software\\GoliathGames\\Garioth"; // Path where settings are stored in the registry

    // Open or create the registry key
    if (RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        // Save fullscreen setting
        RegSetValueExA(hKey, "Fullscreen", 0, REG_DWORD, (const BYTE*)&fullscreen, sizeof(fullscreen));

        // Save resolution as a string (e.g., "1920x1080")
        RegSetValueExA(hKey, "Resolution", 0, REG_SZ, (const BYTE*)resolution.c_str(), resolution.size() + 1);

        // Close the registry key
        RegCloseKey(hKey);
    }
}

// Function to load settings from the registry
void LoadSettingsFromRegistry(bool& fullscreen, std::string& resolution) {
    HKEY hKey;
    const char* regPath = "Software\\GoliathGames\\Garioth";

    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type;
        DWORD size;

        size = sizeof(fullscreen);
        if (RegQueryValueExA(hKey, "Fullscreen", 0, &type, (LPBYTE)&fullscreen, &size) != ERROR_SUCCESS) {
            fullscreen = false; // Default to false if not found
        }

        size = 256;
        char resBuffer[256];
        if (RegQueryValueExA(hKey, "Resolution", 0, &type, (LPBYTE)resBuffer, &size) == ERROR_SUCCESS) {
            resolution = std::string(resBuffer);
        } else {
            resolution = "1920x1080"; // Default if not found
        }

        RegCloseKey(hKey);
    }
}

// Modern OpenGL Shader source code for rendering a textured quad
const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main()
    {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    })";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;
    uniform sampler2D texture1;
    void main()
    {
        FragColor = texture(texture1, TexCoord);
    })";

GLuint LoadTexture(const char* filepath) {
    int width, height, nrChannels;
    std::cout << "Loading texture from: " << filepath << std::endl;

    // Load the image
    unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture at " << filepath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID); // Generate a texture object
    glBindTexture(GL_TEXTURE_2D, textureID); // Bind the texture

    // Set texture wrapping/filtering options (these are just defaults, you can modify these)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrap horizontally
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrap vertically
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Minification filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Magnification filter

    // Upload the texture data (flip the image vertically to correct orientation)
    glTexImage2D(GL_TEXTURE_2D, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB), width, height, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D); // Generate mipmaps for the texture

    // Free image data after use
    stbi_image_free(data);

    return textureID;
}

GLuint setupQuad() {
    // Define the vertices for a textured quad
    GLfloat vertices[] = {
        // Positions         // Texture coordinates
        1.0f,  1.0f,  0.0f, 1.0f, 0.0f,
       -1.0f,  1.0f,  0.0f, 0.0f, 0.0f,
       -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        1.0f, -1.0f,  0.0f, 1.0f, 1.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

void DrawBackgroundTexture(GLuint VAO, GLuint shaderProgram, GLuint textureID) {
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void checkShaderCompile(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void checkProgramLink(GLuint program) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

GLuint createShaderProgram() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompile(vertexShader);

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompile(fragmentShader);

    // Link shaders into a program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramLink(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void window_focus_callback(GLFWwindow* window, int focused) {
    if (focused){
        std::cout << "Window gained focus, resetting custom cursor." << std::endl;
        glfwSetCursor(window, customCursor);
    }
}

std::pair<int, int> ParseResolution(const std::string& resolutionStr) {
    std::stringstream ss(resolutionStr);
    int resWidth, resHeight;
    char x;  // To absorb the 'x' character
    ss >> resWidth >> x >> resHeight;
    return {resWidth, resHeight};
}


void ApplySettings(GLFWwindow* window) {
    if (currentResolutionIndex >= 0 && currentResolutionIndex < resolutions.size()) {
        // Parse resolution string into width and height
        std::pair<int, int> res = ParseResolution(resolutions[currentResolutionIndex]);
        int width = res.first;
        int height = res.second;

        if (fullscreen) {
            // Switch to fullscreen mode
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            // Set windowed mode with the selected resolution
            glfwSetWindowMonitor(window, NULL, 100, 100, width, height, GLFW_DONT_CARE);

            // Center the window on the screen
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            int windowPosX = (mode->width - width) / 2;
            int windowPosY = (mode->height - height) / 2;
            glfwSetWindowPos(window, windowPosX, windowPosY);
        }

        std::cout << "Resolution applied: " << width << "x" << height << std::endl;
    }

    // Save the applied settings to the registry
    SaveSettingsToRegistry(fullscreen, resolutions[currentResolutionIndex]);

    std::cout << "Settings Applied: Fullscreen - " << (fullscreen ? "ON" : "OFF") 
              << ", Resolution - " << resolutions[currentResolutionIndex] << std::endl;
}

// Function to read the Privacy Policy from a file
std::string ReadLegalFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return "Failed to open the file.";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

// Function to process the Privacy Policy text
std::string ProcessLegalPolicyText(const std::string& text)
{
    std::string processedText;
    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == '\\' && i + 1 < text.size() && text[i + 1] == 'n')
        {
            processedText += '\n';
            ++i; // Skip the 'n' character
        }
        else
        {
            processedText += text[i];
        }
    }
    return processedText;
}



void ShowSettingsMenu(GLFWwindow* window) {
    // Load settings from registry before showing the menu
    LoadSettingsFromRegistry(fullscreen, resolution);

    // Find the resolution index to select the correct one in the dropdown
    currentResolutionIndex = -1;  // Default to -1 if not found
    for (int i = 0; i < resolutions.size(); ++i) {
        if (resolutions[i] == resolution) {
            currentResolutionIndex = i;
            break;
        }
    }

    if (currentResolutionIndex == -1) {
        currentResolutionIndex = 0;  // Default to the first resolution if not found
    }

    // Load resolution strings into resolutionCStrs once
    if (!resolutionsLoaded) {
        resolutionCStrs.clear();
        for (const auto& res : resolutions) {
            resolutionCStrs.push_back(res.c_str());
        }
        resolutionsLoaded = true;
    }

    if (ImGui::BeginPopupModal("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Settings");

        if (ImGui::CollapsingHeader("Video")) {
            if (ImGui::Checkbox("Enable Fullscreen", &fullscreen)) {
                std::cout << "Fullscreen Enabled: " << fullscreen << std::endl;
            }

            // Show the current resolution
            ImGui::Text("Resolution");
            ImGui::SameLine();
            ImGui::Combo("##resolution", &currentResolutionIndex, resolutionCStrs.data(), resolutionCStrs.size());
        }

        ImGui::Spacing();
        if (ImGui::Button("Apply", ImVec2(120, 0))) {
            resolution = resolutions[currentResolutionIndex];  // Save the selected resolution
            SaveSettingsToRegistry(fullscreen, resolution);
            ApplySettings(window);
        }

        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}



int main() {
    const std::string privacyPolicyFilePath = "assets/legal/privacy.txt";
    const std::string termsOfServiceFilePath = "assets/legal/tos.txt";

    std::string rawPrivacyPolicyText = ReadLegalFromFile(privacyPolicyFilePath);
    std::string privacyPolicyText = ProcessLegalPolicyText(rawPrivacyPolicyText);
    std::string rawTermsOfServiceText = ReadLegalFromFile(termsOfServiceFilePath);
    std::string termsOfServiceText = ProcessLegalPolicyText(rawTermsOfServiceText);


    //   // Check if the directory exists
    // if (!DirectoryUtils::directoryExists(path)) {
    //     std::cout << "Directory does not exist, creating it..." << std::endl;
    //     bool created = DirectoryUtils::createDirectory(path);
    //     if (created) {
    //         std::cout << "Directory created successfully!" << std::endl;
    //     } else {
    //         std::cerr << "Failed to create directory." << std::endl;
    //     }
    // } else {
    //     std::cout << "Directory already exists!" << std::endl;
    // }

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 0);

    // Load saved settings from the registry
    LoadSettingsFromRegistry(fullscreen, resolution);

    // Parse the resolution string to get the width and height
    std::pair<int, int> resolutionDimensions = ParseResolution(resolution);
    int resWidth = resolutionDimensions.first;
    int resHeight = resolutionDimensions.second;

    // Create window based on fullscreen setting
    GLFWwindow* window = nullptr;
    if (fullscreen) {
        // Switch to fullscreen mode
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        window = glfwCreateWindow(mode->width, mode->height, "Garioth Login", glfwGetPrimaryMonitor(), NULL);
    } else {
        // Create a regular window with the saved resolution
        window = glfwCreateWindow(resWidth, resHeight, "Garioth Login", NULL, NULL);
    }
    
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Make Icon
    std::string iconPath = "assets/images/icon2.png";  // Replace with the actual path to your icon
    std::cout << "Loading icon from: " << iconPath << std::endl;
    int iconWidth, iconHeight, iconChannels;
    unsigned char* image = stbi_load(iconPath.c_str(), &iconWidth, &iconHeight, &iconChannels, 4);

    if (image) {
        GLFWimage icon;
        icon.width = iconWidth;
        icon.height = iconHeight;
        icon.pixels = image;
        glfwSetWindowIcon(window, 1, &icon);
        stbi_image_free(image);
    } else {
        std::cerr << "Failed to load icon image" << std::endl;
    }

    AudioManager audioManager;

    if (!audioManager.init()) {
        std::cerr << "Failied to initialize AudioManager" << std::endl;
        return -1;
    }

    //Start playing bg music
    std::string musicFilePath = "assets/audio/menu.mp3";
    if (!audioManager.playMusic(musicFilePath)) {
        std::cerr << "Failed to play music" << std::endl;
    }

    // Load background texture
    GLuint bgTexture = LoadTexture("assets/images/bg.jpg");

    // Set cursor mode to normal
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);    

    if (customCursor) {
        glfwSetCursor(window, customCursor);
        std::cout << "Custom cursor set successfully!" << std::endl;
    } else {
        std::cerr << "Custom cursor not set!" << std::endl;
    }

    glfwSetWindowFocusCallback(window, window_focus_callback);

    // Check for texture loading errors
    if (bgTexture == 0) {
        std::cerr << "Texture loading failed!" << std::endl;
        return -1;
    }

    // Setup shaders and quad
    GLuint shaderProgram = createShaderProgram();
    GLuint VAO = setupQuad();

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.Fonts->AddFontFromFileTTF("assets/fonts/UncialAntiqua-Regular.ttf", 24.0f);

    // Fantasy theme styling
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.04f, 0.02f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.8f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.1f, 0.05f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.6f, 0.3f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.8f, 0.4f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.5f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.8f, 0.7f, 0.4f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Login/Register fields variables
    char username[128] = "";
    char password[128] = "";
    char confirmPassword[128] = "";
    char email[128] = "";

    bool isRegistering = false;
    bool isAuthenticated = false;
    
    bool showSettingsModal = false;
    bool showExitModal = false;
    bool privacyPolicyAccepted = false;
    bool termsOfServiceAccepted = false;
    bool showPrivacyPolicy = false;
    bool showTermsOfService = false;


    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    // Create a custom cursor and get the cursor, width, and height
    auto [customCursor, width, height] = CreateCustomCursor("assets/images/cursor.png", 0, 0);

    // Optionally resize the cursor if needed

    int cursorWidth = 32;
    int cursorHeight = 32;
    int hotspotX = 16;
    int hotspotY = 16;

    customCursor = ResizeCursor(customCursor, "assets/images/cursor.png", cursorWidth, cursorHeight, hotspotX, hotspotY);
    // customCursor = ResizeCursor(customCursor, "assets/images/cursor.png", 32, 32, 16, 16);

    // Now you can use customCursor, width, and height as needed


    if (customCursor) {
        glfwSetCursor(window, customCursor);
        std::cout << "Custom cursor set successfully!" << std::endl;
    } else {
        std::cerr << "Custom cursor not set!" << std::endl;
    }

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Render the background texture
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        DrawBackgroundTexture(VAO, shaderProgram, bgTexture);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // SETTINGS/EXIT START

        // Button Size and Padding for Layout
ImVec2 buttonSize(120, 40);  // Button size with increased height
ImVec2 padding(10, 10);  // Padding to the right and top

// Add extra padding to the right of the "Exit" button
float extraRightPadding = 20.0f;

// Set window flags to be non-debug, without a title bar, and transparent
ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;

// Position and size of the window in the top-right corner, with extra right padding
ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - buttonSize.x * 2 - padding.x - extraRightPadding, padding.y));
ImGui::SetNextWindowSize(ImVec2(buttonSize.x * 2 + padding.x * 2 + extraRightPadding, buttonSize.y + padding.y * 2));

ImGui::Begin("TopRightButtons", NULL, window_flags);
{
    // Settings Button
    if (ImGui::Button("Settings", buttonSize)) {
        showSettingsModal = true;
    }

    ImGui::SameLine();

    // Exit Button
    if (ImGui::Button("Exit", buttonSize)) {
        showExitModal = true;
    }
}
ImGui::End();

// Exit Modal
if (showExitModal) {
    ImGui::OpenPopup("Exit");
}

if (ImGui::BeginPopupModal("Exit", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Are you sure you want to exit?");
    ImGui::Separator();

    if (ImGui::Button("Yes", ImVec2(120, 0))) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);  // Close the window
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("No", ImVec2(120, 0))) {
        ImGui::CloseCurrentPopup();
        showExitModal = false;
    }

    ImGui::EndPopup();
}

// Settings Modal
if (showSettingsModal) {
    ImGui::OpenPopup("Settings");
}

if (ImGui::BeginPopupModal("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Settings");

    // Load resolution strings and set the current resolution
    if (!resolutionsLoaded) {
        GetAvailableResolutions(resolutions);  // Assuming this function loads available resolutions
        for (const auto& res : resolutions) {
            resolutionCStrs.push_back(res.c_str());
        }
        resolutionsLoaded = true;
    }

    if (ImGui::CollapsingHeader("Video")) {
        // Fullscreen Toggle
        if (ImGui::Checkbox("Enable Fullscreen", &fullscreen)) {
            std::cout << "Fullscreen Enabled: " << fullscreen << std::endl;
        }

        // Resolution Dropdown
        ImGui::Text("Resolution");
        ImGui::SameLine();
        ImGui::Combo("##resolution", &currentResolution, resolutionCStrs.data(), resolutionCStrs.size());

    }

    ImGui::Spacing();
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 2 * buttonSize.x - padding.x) / 2);

    // Apply Button
    if (ImGui::Button("Apply", ImVec2(120, 0))) {
        // Apply the selected settings
        ApplySettings(window);
    }

    ImGui::SameLine();

    // Close Button
    if (ImGui::Button("Close", ImVec2(120, 0))) {
        showSettingsModal = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
        // SETTINGS/EXIT END

        // Set window position (keeping it the same for both login and register)
        ImGui::SetNextWindowPos(ImVec2((display_w - 400) * 0.5f, (display_h - 500) * 0.5f), ImGuiCond_Always);

        // Set window size based on whether the user is registering or logging in
        if (isRegistering) {
            ImGui::SetNextWindowSize(ImVec2(450, 620), ImGuiCond_Always);
        } else {
            ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Always); // Smaller size for login
        }

        ImGui::Begin("Login / Register", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Garioth - Realm of Legends"); // Gold color for the heading
        ImGui::Separator(); // Adds a separator below the heading

        if (isRegistering) {
            ImGui::Text("Register New Account");

            // Set width for input fields to 80% of the window width
            float windowWidth = ImGui::GetWindowWidth();
            float inputWidth = windowWidth * 0.8f;  // 80% of window width

            ImGui::TextUnformatted("Email");
            ImGui::PushItemWidth(inputWidth);
            ImGui::InputText("##email", email, IM_ARRAYSIZE(email));
            ImGui::PopItemWidth();

            ImGui::TextUnformatted("Username");
            ImGui::PushItemWidth(inputWidth);
            ImGui::InputText("##username", username, IM_ARRAYSIZE(username));
            ImGui::PopItemWidth();

            ImGui::TextUnformatted("Password");
            ImGui::PushItemWidth(inputWidth);
            ImGui::InputText("##password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
            ImGui::PopItemWidth();

            ImGui::TextUnformatted("Confirm Password");
            ImGui::PushItemWidth(inputWidth);
            ImGui::InputText("##confirmPassword", confirmPassword, IM_ARRAYSIZE(confirmPassword), ImGuiInputTextFlags_Password);
            ImGui::PopItemWidth();

            ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Add spacing above the button

            // Add checkboxes for Privacy Policy and Terms of Service
            ImGui::Checkbox("I agree to the Privacy Policy", &privacyPolicyAccepted);
            ImGui::Checkbox("I agree to the Terms of Service", &termsOfServiceAccepted);
            
            ImGui::Dummy(ImVec2(0.0f, 15.0f));

            // Button to show Privacy Policy
            if (ImGui::Button("View Privacy Policy")) {
                showPrivacyPolicy = true;
            }

            // Button to show Terms of Service
            if (ImGui::Button("View Terms of Service")) {
                showTermsOfService = true;
            }

            // Disable Register button if the checkboxes are not checked
            bool canRegister = privacyPolicyAccepted && termsOfServiceAccepted;
            ImGui::Dummy(ImVec2(0, 30.0f));
            // Center the "Register" button
            ImGui::SetCursorPosX((windowWidth - 150) * 0.5f);
            if (ImGui::Button("Register", ImVec2(150, 0))) {
                if (strcmp(password, confirmPassword) == 0) {
                    if (canRegister) {
                        std::cout << "Registered: " << username << std::endl;
                    } else {
                        std::cout << "You must accept the Privacy Policy and Terms of Service to register." << std::endl;
                    }
                } else {
                    std::cout << "Passwords do not match!" << std::endl;
                }
            }

            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50);
            ImGui::SetCursorPosX((windowWidth - 250) * 0.5f);
            if (ImGui::Button("Back to login.", ImVec2(250, 0))) {
                isRegistering = false;
            }
        } else {
            // Login Form
            ImGui::Text("Login");

            // Set width for input fields to 80% of the window width
            float windowWidth = ImGui::GetWindowWidth();
            float inputWidth = windowWidth * 0.8f;  // 80% of window width

            ImGui::TextUnformatted("Username");
            ImGui::PushItemWidth(inputWidth);
            ImGui::InputText("##username", username, IM_ARRAYSIZE(username));
            ImGui::PopItemWidth();

            ImGui::TextUnformatted("Password");
            ImGui::PushItemWidth(inputWidth);
            ImGui::InputText("##password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
            ImGui::PopItemWidth();

            ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Add spacing above the button

            // Center the "Login" button
            ImGui::SetCursorPosX((windowWidth - 150) * 0.5f);
            if (ImGui::Button("Login", ImVec2(150, 0))) {
                std::cout << "Logged in: " << username << std::endl;
            }

            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50);
            ImGui::SetCursorPosX((windowWidth - 250) * 0.5f);
            if (ImGui::Button("Don't have an account? Create one.", ImVec2(250, 0))) {
                isRegistering = true;
            }
        }

        // Popup for Privacy Policy
        if (showPrivacyPolicy) {
            ImVec2 windowSize(1000, 800); // Popup size (width, height)
            ImVec2 screenSize = ImGui::GetIO().DisplaySize; // Get screen size

            // Calculate the center of the screen
            ImVec2 windowPos = ImVec2((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);
    
            // Set the position and size of the popup
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
            ImGui::OpenPopup("Privacy Policy");
        }
        if (ImGui::BeginPopupModal("Privacy Policy", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextWrapped("%s", privacyPolicyText.c_str());  // Display the contents of the Privacy Policy
            ImGui::Separator();
            if (ImGui::Button("Close")) {
                showPrivacyPolicy = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Popup for Terms of Service
        if (showTermsOfService) {
            ImVec2 windowSize(1000, 800); // Popup size (width, height)
            ImVec2 screenSize = ImGui::GetIO().DisplaySize; // Get screen size

            // Calculate the center of the screen
            ImVec2 windowPos = ImVec2((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);
    
            // Set the position and size of the popup
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
            ImGui::OpenPopup("Terms of Service");
        }
        if (ImGui::BeginPopupModal("Terms of Service", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextWrapped("%s", termsOfServiceText.c_str());  // Display the contents of the Terms of Service
            ImGui::Separator();
            if (ImGui::Button("Close")) {
                showTermsOfService = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();

        // Render the frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    if (customCursor) {
        glfwDestroyCursor(customCursor);
    }
    glfwDestroyWindow(window);
    audioManager.stopMusic();
    glfwTerminate();
    return 0;
}