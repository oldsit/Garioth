#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    //Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Garioth Login", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to intialize GLAD" << std::endl;
        return -1;
    }

    // Setup OpenGL.
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 800, 600);

    while (!glfwWindowShouldClose(window)) {
        //Clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Swap buffers
        glfwSwapBuffers(window);

        //Poll events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}