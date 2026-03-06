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
  return body;
}

std::vector<Body> createSolarSystem() {
  std::vector<Body> bodies(3); // pre-size for sun + mercury + venus
  #pragma omp parallel sections
  {
    #pragma omp section
    {
      // Sun
      bodies[0] = createBody(
          glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, 0.0f),
          332800.0f,                    // mass in Earth units
          32.8f,                        // radius scaled to Earth
          glm::vec3(1.0f, 1.0f, 0.0f)  // yellow color
      );
    }
    #pragma omp section
    {
      // Mercury: 0.387 AU, mass 0.0553 Earth, circular orbit in XZ plane
      // Hill sphere ~0.22 sim units, radius kept below that
      float mercury_dist = 58.05f;                             // 0.387 * 150
      float mercury_v    = sqrt(G * 332800.0f / mercury_dist);
      bodies[1] = createBody(
          glm::vec3(mercury_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -mercury_v),
          0.0553f,                              // mass in Earth units
          0.15f,                                // below Hill sphere ~0.22
          glm::vec3(0.6f, 0.6f, 0.6f)          // grey
      );
    }
    #pragma omp section
    {
      // Venus: 0.723 AU, mass 0.815 Earth, slightly larger than Mercury visually
      // Hill sphere ~1.0 sim units, radius kept below that
      float venus_dist = 108.45f;                             // 0.723 * 150
      float venus_v    = sqrt(G * 332800.0f / venus_dist);
      bodies[2] = createBody(
          glm::vec3(venus_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -venus_v),
          0.815f,                               // mass in Earth units
          0.85f,                                // below Hill sphere ~1.0, bigger than Mercury
          glm::vec3(0.9f, 0.75f, 0.45f)        // yellowish-orange
      );
    }
  }
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

  // Record trail positions — separate parallel loop, each thread owns its own body
  #pragma omp parallel for schedule(static)
  for (size_t i = 0; i < n; ++i) {
    bodies[i].trail.push_back(bodies[i].position);
    if (bodies[i].trail.size() > TRAIL_LENGTH)
      bodies[i].trail.pop_front();
  }
}

void drawBody(const Body& body) {
  // Draw orbit trail — unlit fading line
  int total = body.trail.size();
  if (total > 1) {
    glDisable(GL_LIGHTING);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < total; i++) {
      float alpha = (float)i / total; // fade from transparent at tail to full at head
      glColor4f(body.color.r, body.color.g, body.color.b, alpha);
      glVertex3f(body.trail[i].x, body.trail[i].y, body.trail[i].z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
  }

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
