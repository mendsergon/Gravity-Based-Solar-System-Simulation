#include <iostream>
#include <vector>
#include <cmath>
#include "functions.h"
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

// Callback for window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Zoom factor — 1.0 is fully zoomed out, smaller values zoom in
float zoomFactor   = 1.0f;

// Camera target — shifts toward cursor on zoom, X and Z since planets orbit in XZ plane
float camTargetX   = 0.0f;
float camTargetY   = 0.0f;
float camTargetZ   = 0.0f;

// Current scene size — updated each frame, used in scroll callback
float sceneMaxDist = 1.0f;

// Matrices stored each frame — used in scroll callback to unproject cursor to world space
GLdouble g_modelview[16];
GLdouble g_projection[16];
GLint    g_viewport[4];

// Scroll callback — zoom toward mouse position at time of scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Unproject cursor onto near and far plane to get world ray
    GLdouble wx0, wy0, wz0;
    GLdouble wx1, wy1, wz1;
    gluUnProject(xpos, g_viewport[3] - ypos, 0.0,
                 g_modelview, g_projection, g_viewport,
                 &wx0, &wy0, &wz0);
    gluUnProject(xpos, g_viewport[3] - ypos, 1.0,
                 g_modelview, g_projection, g_viewport,
                 &wx1, &wy1, &wz1);

    // Intersect ray with orbital plane Y=0
    double dy = wy1 - wy0;
    if (fabs(dy) < 1e-6) return; // ray parallel to orbital plane — skip
    double t     = -wy0 / dy;
    float worldX = (float)(wx0 + t * (wx1 - wx0));
    float worldZ = (float)(wz0 + t * (wz1 - wz0));

    // Apply zoom — multiplicative so each scroll is a fixed percentage at any scale
    float oldZoom = zoomFactor;
    zoomFactor *= powf(0.9f, (float)yoffset);
    if (zoomFactor < 0.00001f) zoomFactor = 0.00001f; // clamp — close enough to see moons
    if (zoomFactor > 1.0f)     zoomFactor = 1.0f;     // clamp — full solar system view

    // Zoom toward world point — move camera target toward cursor point proportional to zoom change
    float ratio = zoomFactor / oldZoom;
    camTargetX  = worldX + (camTargetX - worldX) * ratio;
    camTargetZ  = worldZ + (camTargetZ - worldZ) * ratio;

    // Reset target when fully zoomed out
    if (zoomFactor >= 1.0f) {
        camTargetX = 0.0f;
        camTargetY = 0.0f;
        camTargetZ = 0.0f;
    }
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
    glfwSetScrollCallback(window, scroll_callback);

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
    GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };  // white light
    GLfloat light_ambient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };  // small ambient
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // specular
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Create bodies
    std::vector<Body> bodies = createSolarSystem();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Dynamically fit all bodies in view based on furthest body
        float maxDist = 0.0f;
        for (const auto& body : bodies)
            maxDist = std::max(maxDist, glm::length(body.position));
        sceneMaxDist = maxDist;    // keep scroll callback in sync with scene size
        g_zoomFactor = zoomFactor; // pass zoom to drawBody for sphere detail scaling
        float camDist = maxDist * 2.0f * zoomFactor; // camera distance drives zoom

        // Camera position passed to drawBody for minimum apparent size calculation
        g_camX = camTargetX;
        g_camY = camDist * 0.3f + camTargetY;
        g_camZ = camTargetZ + camDist;

        // Update projection each frame to match scene size
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Near and far both scale with camDist — preserves depth precision at all zoom levels
        gluPerspective(45.0, 800.0/600.0, camDist * 0.001f, camDist * 1000.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Camera pulled back to fit the full solar system in view, target shifts on scroll
        gluLookAt(g_camX, g_camY, g_camZ,
                  camTargetX, camTargetY, camTargetZ,
                  0.0, 1.0, 0.0);

        // Store matrices each frame so scroll callback can unproject cursor to world space
        glGetDoublev(GL_MODELVIEW_MATRIX,  g_modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, g_projection);
        glGetIntegerv(GL_VIEWPORT,         g_viewport);

        // Light position set after gluLookAt each frame so it transforms correctly
        GLfloat light_pos[] = { 0.0f, 0.0f, 0.0f, 1.0f };  // light at sun position
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

        // Multiple physics steps per frame — stable orbits at same visual speed
        for (int step = 0; step < 10; ++step) {
            updateGravity(bodies, 0.001f);
        }

        // Draw all bodies
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
