#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <glm/glm.hpp>
#include <vector>
#include <deque>

// Gravitational constant (arbitrary scale for simulation)
const float G = 6.67430e-3f; // scaled down for visual simulation

// Trail length — how many past positions to keep per body
const int TRAIL_LENGTH = 200;

// Zoom level passed from main — used to scale sphere detail in drawBody
extern float g_zoomFactor;

// Camera position passed from main — used to enforce minimum apparent size in drawBody
extern float g_camX, g_camY, g_camZ;

// Struct representing a celestial body
struct Body {
  glm::vec3 position;          // current position
  glm::vec3 velocity;          // current velocity
  float mass;                  // mass (affects gravity)
  float radius;                // for rendering
  glm::vec3 color;             // RGB color
  std::deque<glm::vec3> trail; // position history for orbit trail
  bool hasRing;                // whether to draw a ring around this body
  float ringInnerRadius;       // inner radius of ring
  float ringOuterRadius;       // outer radius of ring
};

// Physics update: compute gravitational forces and update velocities & positions
void updateGravity(std::vector<Body>& bodies, float deltaTime);

// Rendering: draw a sphere at body's current position
void drawBody(const Body& body);

// Utility: create a single celestial body
Body createBody(const glm::vec3& position,
                const glm::vec3& velocity,
                float mass, float radius,
                const glm::vec3& color);

// Factory: create the full solar system
std::vector<Body> createSolarSystem();

#endif
