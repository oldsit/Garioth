#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Include ImGui headers
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

static int PasswordCallback(ImGuiInputTextCallbackData* data) {
    // Replace each character with a bullet point
    for (int i = 0; i < data->BufTextLen; i++) {
        data->Buf[i] = '•';
    }
    return 0;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Garioth Login", NULL, NULL);
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

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Customize style for fantasy theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.7f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.3f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.25f, 0.25f, 1.0f);

    // Initialize ImGui GLFW and OpenGL bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    char email[128] = "";
    char username[128] = "";
    char password[128] = "";
    bool showPassword = false;
    bool loginClicked = false;
    bool registerClicked = false;
    bool isLogin = true; // Toggle between login and registration

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Clear screen
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f); // Darker background
        glClear(GL_COLOR_BUFFER_BIT);

        // Start Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create UI
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        ImGui::SetNextWindowPos(ImVec2((display_w - 300) / 2, (display_h - 250) / 2));
        ImGui::SetNextWindowSize(ImVec2(300, 250));
        ImGui::Begin(isLogin ? "Garioth Login" : "Garioth Register", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (isLogin) {
            ImGui::Text("Username:");
            ImGui::InputText("##username", username, IM_ARRAYSIZE(username));

            ImGui::Text("Password:");
            ImGui::InputText("##password", password, IM_ARRAYSIZE(password), showPassword ? 0 : ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CallbackAlways, PasswordCallback);

            ImGui::Checkbox("Show Password", &showPassword);

            if (ImGui::Button("Login")) {
                loginClicked = true;
                std::cout << "Logging in with Username: " << username << " and Password: " << password << std::endl;
                // Clear textboxes
                username[0] = '\0';
                password[0] = '\0';
            }

            if (ImGui::Button("Register")) {
                isLogin = false; // Switch to registration form
                // Clear textboxes
                username[0] = '\0';
                password[0] = '\0';
            }
        } else {
            ImGui::Text("Email:");
            ImGui::InputText("##email", email, IM_ARRAYSIZE(email));

            ImGui::Text("Username:");
            ImGui::InputText("##username", username, IM_ARRAYSIZE(username));

            ImGui::Text("Password:");
            ImGui::InputText("##password", password, IM_ARRAYSIZE(password), showPassword ? 0 : ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CallbackAlways, PasswordCallback);

            ImGui::Checkbox("Show Password", &showPassword);

            if (ImGui::Button("Register")) {
                registerClicked = true;
                std::cout << "Registering with Email: " << email << ", Username: " << username << " and Password: " << password << std::endl;
                // Clear textboxes
                email[0] = '\0';
                username[0] = '\0';
                password[0] = '\0';
            }

            if (ImGui::Button("Back to Login")) {
                isLogin = true; // Switch back to login form
                // Clear textboxes
                email[0] = '\0';
                username[0] = '\0';
                password[0] = '\0';
            }
        }

        ImGui::End();

        // Render ImGui frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui and GLFW
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
