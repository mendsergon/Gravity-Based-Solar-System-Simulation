#include <iostream>
#include <vector>
#include "functions.h"
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

// Callback for window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Set OpenGL version 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Solar System Test", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable depth test for proper 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Enable alpha blending for trail fade effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable lighting 
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);  // Light 0

    // Enable GL_COLOR_MATERIAL so glColor3f() actually affects the lit material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Set light properties
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };   // white light
    GLfloat light_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };   // small ambient
    GLfloat light_specular[]= { 1.0f, 1.0f, 1.0f, 1.0f };   // specular
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Setup 3D perspective — far plane extended for full solar system scale
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0/600.0, 0.1, 10000.0);
    glMatrixMode(GL_MODELVIEW);

    // Create bodies 
    std::vector<Body> bodies = createSolarSystem();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Camera pulled way back to fit the full solar system in view
        gluLookAt(0.0, 300.0, 800.0,
                  0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);

        // Light position set after gluLookAt each frame so it transforms correctly
        GLfloat light_pos[] = { 0.0f, 0.0f, 0.0f, 1.0f };  // light at sun position
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

        // Update physics 
        updateGravity(bodies, 0.01f);

        //  Draw all bodies 
        for (auto& body : bodies) {
            drawBody(body);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
