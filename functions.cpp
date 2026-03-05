#include "functions.h"
#include <glm/gtc/constants.hpp> // Math constants (PI)
#include <glm/gtc/matrix_transform.hpp> // For vector/matrix transformation
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <omp.h> // OpenMP for parallel physics calculations

// Create a single celestial body with given parameters
Body createBody(const glm::vec3& position,
                const glm::vec3& velocity,
                float mass, float radius,
                const glm::vec3& color)
{
  Body body;
  body.position = position;
  body.velocity = velocity;
  body.mass = mass;
  body.radius = radius;
  body.color = color;
  body.moons.clear(); // Start with no moons
  return body;
}

// Create Solar System
std::vector<Body> createSolarSystem() {
  std::vector<Body> bodies;

  // Sun
  Body sun = createBody(
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      332800.0f,                    // mass in Earth units
      32.8f,                        // radius scaled to Earth
      glm::vec3(1.0f, 1.0f, 0.0f)   // yellow color
      );

  bodies.push_back(sun);

  return bodies;
}

void updateGravity(std::vector<Body>& bodies, float deltaTime) {
  size_t n = bodies.size();

  // Temporary storage for forces
  std::vector<glm::vec3> forces(n, glm::vec3(0.0f));

  // Compute forces (parallelized over bodies)
#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < n; ++i) {
    glm::vec3 totalForce(0.0f);
    for (size_t j = 0; j < n; ++j) {
      if (i == j) continue;

      glm::vec3 dir = bodies[j].position - bodies[i].position;
      float distSqr = glm::dot(dir, dir) + 1e-4f;
      float dist = sqrt(distSqr);
      glm::vec3 force = (G * bodies[i].mass * bodies[j].mass / distSqr) * (dir / dist);

      totalForce += force;
    }
    forces[i] = totalForce;
  }

  // Update velocities and positions
  #pragma omp parallel for schedule(static)
  for (size_t i = 0; i < n; ++i) {
    glm::vec3 acceleration = forces[i] / bodies[i].mass;
    bodies[i].velocity += acceleration * deltaTime;
    bodies[i].position += bodies[i].velocity * deltaTime;
  }
}

void drawBody(const Body& body) {
  GLUquadric* quad = gluNewQuadric();
  glPushMatrix();
  glTranslatef(body.position.x, body.position.y, body.position.z);
  glColor3f(body.color.r, body.color.g, body.color.b);

  // Make the sun glow — emissive so it's fully bright regardless of lighting
  GLfloat emissive[] = { body.color.r, body.color.g, body.color.b, 1.0f };
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive);

  gluSphere(quad, body.radius, 20, 20); // radius, slices, stacks

  // Reset emissive so future planets are shaded normally
  GLfloat no_emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, no_emissive);

  glPopMatrix();
  gluDeleteQuadric(quad);
}
