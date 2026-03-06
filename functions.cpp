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
  body.hasRing = false;        // no ring by default
  body.ringInnerRadius = 0.0f;
  body.ringOuterRadius = 0.0f;
  return body;
}

std::vector<Body> createSolarSystem() {
  std::vector<Body> bodies(30); // pre-size
  #pragma omp parallel sections
  {
    #pragma omp section
    {
      // Sun
      bodies[0] = createBody(
          glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, 0.0f),
          332800.0f,                    // mass in Earth units
          32.8f,                        // intentionally compressed or it dwarfs everything
          glm::vec3(1.0f, 1.0f, 0.0f)  // yellow
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
          1.9f,                                 // 0.383 Earth radii * 5
          glm::vec3(0.6f, 0.6f, 0.6f)          // grey
      );
    }
    #pragma omp section
    {
      // Venus: 0.723 AU, mass 0.815 Earth, circular orbit in XZ plane
      // Hill sphere ~1.0 sim units, radius kept below that
      float venus_dist = 108.45f;                             // 0.723 * 150
      float venus_v    = sqrt(G * 332800.0f / venus_dist);
      bodies[2] = createBody(
          glm::vec3(venus_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -venus_v),
          0.815f,                               // mass in Earth units
          4.75f,                                // 0.950 Earth radii * 5
          glm::vec3(0.9f, 0.75f, 0.45f)        // yellowish-orange
      );
    }
    #pragma omp section
    {
      // Earth: 1.0 AU, mass 1.0 Earth, circular orbit in XZ plane
      // Hill sphere ~1.5 sim units, radius kept below that
      float earth_dist = 150.0f;                              // 1.0 * 150
      float earth_v    = sqrt(G * 332800.0f / earth_dist);
      bodies[3] = createBody(
          glm::vec3(earth_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -earth_v),
          1.0f,                                 // mass in Earth units
          5.0f,                                 // 1.0 Earth radii * 5 — anchor
          glm::vec3(0.2f, 0.4f, 1.0f)          // blue
      );
    }
    #pragma omp section
    {
      // Moon: 0.00257 AU from Earth, mass 0.0123 Earth, orbits Earth in XZ plane
      // sits within Earth Hill sphere (~1.5 sim units), outside Earth radius
      float earth_dist = 150.0f;                              // 1.0 * 150
      float earth_v    = sqrt(G * 332800.0f / earth_dist);
      float moon_dist  = 1.2f;                               // within Hill sphere ~1.5
      float moon_v     = sqrt(G * 1.0f / moon_dist);        // orbital velocity around Earth
      bodies[4] = createBody(
          glm::vec3(earth_dist + moon_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(earth_v + moon_v)),
          0.0123f,                                            // mass in Earth units
          1.37f,                                              // 0.273 Earth radii * 5
          glm::vec3(0.7f, 0.7f, 0.7f)                        // grey
      );
    }
    #pragma omp section
    {
      // Mars: 1.524 AU, mass 0.107 Earth, circular orbit in XZ plane
      // Hill sphere ~1.086 sim units, radius kept below that
      float mars_dist = 228.6f;                              // 1.524 * 150
      float mars_v    = sqrt(G * 332800.0f / mars_dist);
      bodies[5] = createBody(
          glm::vec3(mars_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -mars_v),
          0.107f,                               // mass in Earth units
          2.66f,                                // 0.532 Earth radii * 5
          glm::vec3(0.8f, 0.3f, 0.1f)          // red-orange
      );
    }
    #pragma omp section
    {
      // Phobos: 0.0000626 AU from Mars, mass 1.0659e-8 Earth, orbits Mars in XZ plane
      // sits within Mars Hill sphere (~1.086 sim units), outside Mars radius
      float mars_dist   = 228.6f;                            // 1.524 * 150
      float mars_v      = sqrt(G * 332800.0f / mars_dist);
      float phobos_dist = 0.3f;                              // within Hill sphere ~1.086
      float phobos_v    = sqrt(G * 0.107f / phobos_dist);   // orbital velocity around Mars
      bodies[6] = createBody(
          glm::vec3(mars_dist + phobos_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(mars_v + phobos_v)),
          1.0659e-8f,                                         // mass in Earth units
          0.5f,                                               // floored for visibility, true size sub-pixel
          glm::vec3(0.6f, 0.5f, 0.4f)                        // dark grey-brown
      );
    }
    #pragma omp section
    {
      // Deimos: 0.000157 AU from Mars, mass 1.4762e-9 Earth, orbits Mars in XZ plane
      // sits within Mars Hill sphere (~1.086 sim units), outside Phobos orbit
      float mars_dist   = 228.6f;                            // 1.524 * 150
      float mars_v      = sqrt(G * 332800.0f / mars_dist);
      float deimos_dist = 0.6f;                              // within Hill sphere ~1.086, outside Phobos
      float deimos_v    = sqrt(G * 0.107f / deimos_dist);   // orbital velocity around Mars
      bodies[7] = createBody(
          glm::vec3(mars_dist + deimos_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(mars_v + deimos_v)),
          1.4762e-9f,                                         // mass in Earth units
          0.4f,                                               // floored for visibility, smaller than Phobos
          glm::vec3(0.5f, 0.45f, 0.35f)                      // dark grey-brown, slightly lighter
      );
    }
    #pragma omp section
    {
      // Jupiter: 5.203 AU, mass 317.8 Earth, circular orbit in XZ plane
      // Hill sphere ~53.3 sim units, radius kept below that
      float jupiter_dist = 780.45f;                            // 5.203 * 150
      float jupiter_v    = sqrt(G * 332800.0f / jupiter_dist);
      bodies[8] = createBody(
          glm::vec3(jupiter_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -jupiter_v),
          317.8f,                               // mass in Earth units
          56.05f,                               // 11.21 Earth radii * 5
          glm::vec3(0.8f, 0.7f, 0.5f)          // tan/beige
      );
    }
    #pragma omp section
    {
      // Io: 0.002819 AU from Jupiter, mass 0.015 Earth, orbits Jupiter in XZ plane
      // sits within Jupiter Hill sphere (~53.3 sim units), outside Jupiter radius
      float jupiter_dist = 780.45f;                            // 5.203 * 150
      float jupiter_v    = sqrt(G * 332800.0f / jupiter_dist);
      float io_dist      = 0.423f;                             // 0.002819 * 150
      float io_v         = sqrt(G * 317.8f / io_dist);        // orbital velocity around Jupiter
      bodies[9] = createBody(
          glm::vec3(jupiter_dist + io_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(jupiter_v + io_v)),
          0.015f,                               // mass in Earth units
          1.43f,                                // 0.286 Earth radii * 5
          glm::vec3(0.9f, 0.7f, 0.2f)          // yellow-orange volcanic
      );
    }
    #pragma omp section
    {
      // Europa: 0.004486 AU from Jupiter, mass 0.008 Earth, orbits Jupiter in XZ plane
      // sits within Jupiter Hill sphere (~53.3 sim units), outside Io orbit
      float jupiter_dist = 780.45f;                            // 5.203 * 150
      float jupiter_v    = sqrt(G * 332800.0f / jupiter_dist);
      float europa_dist  = 0.673f;                             // 0.004486 * 150
      float europa_v     = sqrt(G * 317.8f / europa_dist);    // orbital velocity around Jupiter
      bodies[10] = createBody(
          glm::vec3(jupiter_dist + europa_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(jupiter_v + europa_v)),
          0.008f,                               // mass in Earth units
          1.225f,                               // 0.245 Earth radii * 5
          glm::vec3(0.8f, 0.7f, 0.6f)          // pale brown icy
      );
    }
    #pragma omp section
    {
      // Ganymede: 0.007155 AU from Jupiter, mass 0.025 Earth, orbits Jupiter in XZ plane
      // sits within Jupiter Hill sphere (~53.3 sim units), outside Europa orbit
      float jupiter_dist  = 780.45f;                           // 5.203 * 150
      float jupiter_v     = sqrt(G * 332800.0f / jupiter_dist);
      float ganymede_dist = 1.073f;                            // 0.007155 * 150
      float ganymede_v    = sqrt(G * 317.8f / ganymede_dist); // orbital velocity around Jupiter
      bodies[11] = createBody(
          glm::vec3(jupiter_dist + ganymede_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(jupiter_v + ganymede_v)),
          0.025f,                               // mass in Earth units
          2.065f,                               // 0.413 Earth radii * 5, largest moon in solar system
          glm::vec3(0.6f, 0.6f, 0.55f)         // grey-brown
      );
    }
    #pragma omp section
    {
      // Callisto: 0.01259 AU from Jupiter, mass 0.018 Earth, orbits Jupiter in XZ plane
      // sits within Jupiter Hill sphere (~53.3 sim units), outside Ganymede orbit
      float jupiter_dist  = 780.45f;                           // 5.203 * 150
      float jupiter_v     = sqrt(G * 332800.0f / jupiter_dist);
      float callisto_dist = 1.889f;                            // 0.01259 * 150
      float callisto_v    = sqrt(G * 317.8f / callisto_dist); // orbital velocity around Jupiter
      bodies[12] = createBody(
          glm::vec3(jupiter_dist + callisto_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(jupiter_v + callisto_v)),
          0.018f,                               // mass in Earth units
          1.89f,                                // 0.378 Earth radii * 5
          glm::vec3(0.4f, 0.35f, 0.3f)         // dark grey-brown heavily cratered
      );
    }
    #pragma omp section
    {
      // Saturn: 9.537 AU, mass 95.2 Earth, circular orbit in XZ plane
      // Hill sphere ~65.4 sim units, radius kept below that
      float saturn_dist = 1430.55f;                            // 9.537 * 150
      float saturn_v    = sqrt(G * 332800.0f / saturn_dist);
      bodies[13] = createBody(
          glm::vec3(saturn_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -saturn_v),
          95.2f,                                // mass in Earth units
          47.25f,                               // 9.45 Earth radii * 5
          glm::vec3(0.85f, 0.75f, 0.5f)        // pale gold
      );
      // Saturn rings — inner just outside planet, outer scaled to real proportions
      bodies[13].hasRing         = true;
      bodies[13].ringInnerRadius = 52.0f;
      bodies[13].ringOuterRadius = 107.0f;
    }
    #pragma omp section
    {
      // Titan: 0.00817 AU from Saturn, mass 0.0225 Earth, orbits Saturn in XZ plane
      // sits within Saturn Hill sphere (~65.4 sim units), outside Saturn radius
      float saturn_dist = 1430.55f;                            // 9.537 * 150
      float saturn_v    = sqrt(G * 332800.0f / saturn_dist);
      float titan_dist  = 1.226f;                              // 0.00817 * 150
      float titan_v     = sqrt(G * 95.2f / titan_dist);       // orbital velocity around Saturn
      bodies[14] = createBody(
          glm::vec3(saturn_dist + titan_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(saturn_v + titan_v)),
          0.0225f,                              // mass in Earth units
          2.02f,                                // 0.404 Earth radii * 5
          glm::vec3(0.8f, 0.6f, 0.3f)          // orange hazy atmosphere
      );
    }
    #pragma omp section
    {
      // Rhea: 0.00352 AU from Saturn, mass 0.000331 Earth, orbits Saturn in XZ plane
      // sits within Saturn Hill sphere (~65.4 sim units), outside Saturn radius
      float saturn_dist = 1430.55f;                            // 9.537 * 150
      float saturn_v    = sqrt(G * 332800.0f / saturn_dist);
      float rhea_dist   = 0.528f;                              // 0.00352 * 150
      float rhea_v      = sqrt(G * 95.2f / rhea_dist);        // orbital velocity around Saturn
      bodies[15] = createBody(
          glm::vec3(saturn_dist + rhea_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(saturn_v + rhea_v)),
          0.000331f,                            // mass in Earth units
          0.6f,                                 // 0.120 Earth radii * 5
          glm::vec3(0.7f, 0.7f, 0.65f)         // grey-white icy
      );
    }
    #pragma omp section
    {
      // Dione: 0.00252 AU from Saturn, mass 0.000185 Earth, orbits Saturn in XZ plane
      // sits within Saturn Hill sphere (~65.4 sim units), outside Saturn radius
      float saturn_dist = 1430.55f;                            // 9.537 * 150
      float saturn_v    = sqrt(G * 332800.0f / saturn_dist);
      float dione_dist  = 0.378f;                              // 0.00252 * 150
      float dione_v     = sqrt(G * 95.2f / dione_dist);       // orbital velocity around Saturn
      bodies[16] = createBody(
          glm::vec3(saturn_dist + dione_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(saturn_v + dione_v)),
          0.000185f,                            // mass in Earth units
          0.44f,                                // 0.0881 Earth radii * 5
          glm::vec3(0.75f, 0.75f, 0.7f)        // grey-white icy
      );
    }
    #pragma omp section
    {
      // Tethys: 0.00197 AU from Saturn, mass 0.000108 Earth, orbits Saturn in XZ plane
      // sits within Saturn Hill sphere (~65.4 sim units), outside Saturn radius
      float saturn_dist = 1430.55f;                            // 9.537 * 150
      float saturn_v    = sqrt(G * 332800.0f / saturn_dist);
      float tethys_dist = 0.296f;                              // 0.00197 * 150
      float tethys_v    = sqrt(G * 95.2f / tethys_dist);      // orbital velocity around Saturn
      bodies[17] = createBody(
          glm::vec3(saturn_dist + tethys_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(saturn_v + tethys_v)),
          0.000108f,                            // mass in Earth units
          0.41f,                                // 0.0827 Earth radii * 5
          glm::vec3(0.8f, 0.8f, 0.78f)         // bright white icy
      );
    }
    #pragma omp section
    {
      // Enceladus: 0.00159 AU from Saturn, mass 0.0000181 Earth, orbits Saturn in XZ plane
      // sits within Saturn Hill sphere (~65.4 sim units), outside Saturn radius
      float saturn_dist    = 1430.55f;                         // 9.537 * 150
      float saturn_v       = sqrt(G * 332800.0f / saturn_dist);
      float enceladus_dist = 0.239f;                           // 0.00159 * 150
      float enceladus_v    = sqrt(G * 95.2f / enceladus_dist); // orbital velocity around Saturn
      bodies[18] = createBody(
          glm::vec3(saturn_dist + enceladus_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(saturn_v + enceladus_v)),
          0.0000181f,                           // mass in Earth units
          0.2f,                                 // 0.0397 Earth radii * 5, floored for visibility
          glm::vec3(0.9f, 0.9f, 0.95f)         // bright white highly reflective
      );
    }
    #pragma omp section
    {
      // Uranus: 19.19 AU, mass 14.54 Earth, circular orbit in XZ plane
      // Hill sphere ~69.9 sim units, radius kept below that
      float uranus_dist = 2878.5f;                             // 19.19 * 150
      float uranus_v    = sqrt(G * 332800.0f / uranus_dist);
      bodies[19] = createBody(
          glm::vec3(uranus_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -uranus_v),
          14.54f,                               // mass in Earth units
          20.05f,                               // 4.01 Earth radii * 5
          glm::vec3(0.5f, 0.85f, 0.9f)         // pale cyan
      );
    }
    #pragma omp section
    {
      // Miranda: 0.000868 AU from Uranus, mass 0.0000903 Earth, orbits Uranus in XZ plane
      // sits within Uranus Hill sphere (~69.9 sim units), outside Uranus radius
      float uranus_dist  = 2878.5f;                            // 19.19 * 150
      float uranus_v     = sqrt(G * 332800.0f / uranus_dist);
      float miranda_dist = 0.130f;                             // 0.000868 * 150
      float miranda_v    = sqrt(G * 14.54f / miranda_dist);   // orbital velocity around Uranus
      bodies[20] = createBody(
          glm::vec3(uranus_dist + miranda_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(uranus_v + miranda_v)),
          0.0000903f,                           // mass in Earth units
          0.18f,                                // 0.0365 Earth radii * 5, floored for visibility
          glm::vec3(0.6f, 0.6f, 0.6f)          // grey
      );
    }
    #pragma omp section
    {
      // Ariel: 0.00127 AU from Uranus, mass 0.000203 Earth, orbits Uranus in XZ plane
      // sits within Uranus Hill sphere (~69.9 sim units), outside Miranda orbit
      float uranus_dist = 2878.5f;                             // 19.19 * 150
      float uranus_v    = sqrt(G * 332800.0f / uranus_dist);
      float ariel_dist  = 0.191f;                              // 0.00127 * 150
      float ariel_v     = sqrt(G * 14.54f / ariel_dist);      // orbital velocity around Uranus
      bodies[21] = createBody(
          glm::vec3(uranus_dist + ariel_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(uranus_v + ariel_v)),
          0.000203f,                            // mass in Earth units
          0.29f,                                // 0.0578 Earth radii * 5
          glm::vec3(0.7f, 0.7f, 0.68f)         // grey-white icy
      );
    }
    #pragma omp section
    {
      // Umbriel: 0.00177 AU from Uranus, mass 0.000197 Earth, orbits Uranus in XZ plane
      // sits within Uranus Hill sphere (~69.9 sim units), outside Ariel orbit
      float uranus_dist  = 2878.5f;                            // 19.19 * 150
      float uranus_v     = sqrt(G * 332800.0f / uranus_dist);
      float umbriel_dist = 0.266f;                             // 0.00177 * 150
      float umbriel_v    = sqrt(G * 14.54f / umbriel_dist);   // orbital velocity around Uranus
      bodies[22] = createBody(
          glm::vec3(uranus_dist + umbriel_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(uranus_v + umbriel_v)),
          0.000197f,                            // mass in Earth units
          0.29f,                                // 0.0579 Earth radii * 5
          glm::vec3(0.4f, 0.4f, 0.4f)          // dark grey heavily cratered
      );
    }
    #pragma omp section
    {
      // Titania: 0.00292 AU from Uranus, mass 0.000592 Earth, orbits Uranus in XZ plane
      // sits within Uranus Hill sphere (~69.9 sim units), outside Umbriel orbit
      float uranus_dist  = 2878.5f;                            // 19.19 * 150
      float uranus_v     = sqrt(G * 332800.0f / uranus_dist);
      float titania_dist = 0.438f;                             // 0.00292 * 150
      float titania_v    = sqrt(G * 14.54f / titania_dist);   // orbital velocity around Uranus
      bodies[23] = createBody(
          glm::vec3(uranus_dist + titania_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(uranus_v + titania_v)),
          0.000592f,                            // mass in Earth units
          0.394f,                               // 0.0789 Earth radii * 5, largest Uranus moon
          glm::vec3(0.65f, 0.62f, 0.6f)        // grey-brown icy
      );
    }
    #pragma omp section
    {
      // Oberon: 0.00391 AU from Uranus, mass 0.000514 Earth, orbits Uranus in XZ plane
      // sits within Uranus Hill sphere (~69.9 sim units), outside Titania orbit
      float uranus_dist = 2878.5f;                             // 19.19 * 150
      float uranus_v    = sqrt(G * 332800.0f / uranus_dist);
      float oberon_dist = 0.587f;                              // 0.00391 * 150
      float oberon_v    = sqrt(G * 14.54f / oberon_dist);     // orbital velocity around Uranus
      bodies[24] = createBody(
          glm::vec3(uranus_dist + oberon_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(uranus_v + oberon_v)),
          0.000514f,                            // mass in Earth units
          0.376f,                               // 0.0753 Earth radii * 5
          glm::vec3(0.55f, 0.52f, 0.5f)        // dark grey-brown icy
      );
    }
    #pragma omp section
    {
      // Neptune: 30.07 AU, mass 17.15 Earth, circular orbit in XZ plane
      // Hill sphere ~116.0 sim units, radius kept below that
      float neptune_dist = 4510.5f;                            // 30.07 * 150
      float neptune_v    = sqrt(G * 332800.0f / neptune_dist);
      bodies[25] = createBody(
          glm::vec3(neptune_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -neptune_v),
          17.15f,                               // mass in Earth units
          19.45f,                               // 3.89 Earth radii * 5
          glm::vec3(0.2f, 0.3f, 0.9f)          // deep blue
      );
    }
    #pragma omp section
    {
      // Triton: 0.002371 AU from Neptune, mass 0.000359 Earth, orbits Neptune in XZ plane
      // sits within Neptune Hill sphere (~116.0 sim units), outside Neptune radius
      float neptune_dist = 4510.5f;                            // 30.07 * 150
      float neptune_v    = sqrt(G * 332800.0f / neptune_dist);
      float triton_dist  = 0.356f;                             // 0.002371 * 150
      float triton_v     = sqrt(G * 17.15f / triton_dist);    // orbital velocity around Neptune
      bodies[26] = createBody(
          glm::vec3(neptune_dist + triton_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(neptune_v + triton_v)),
          0.000359f,                            // mass in Earth units
          0.674f,                               // 0.1348 Earth radii * 5
          glm::vec3(0.7f, 0.65f, 0.6f)         // pinkish-grey nitrogen ice
      );
    }
    #pragma omp section
    {
      // Proteus: 0.000790 AU from Neptune, mass 0.0000000274 Earth, orbits Neptune in XZ plane
      // sits within Neptune Hill sphere (~116.0 sim units), outside Neptune radius
      float neptune_dist  = 4510.5f;                           // 30.07 * 150
      float neptune_v     = sqrt(G * 332800.0f / neptune_dist);
      float proteus_dist  = 0.119f;                            // 0.000790 * 150
      float proteus_v     = sqrt(G * 17.15f / proteus_dist);  // orbital velocity around Neptune
      bodies[27] = createBody(
          glm::vec3(neptune_dist + proteus_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(neptune_v + proteus_v)),
          0.0000000274f,                        // mass in Earth units
          0.18f,                                // 0.0356 Earth radii * 5, floored for visibility
          glm::vec3(0.35f, 0.35f, 0.35f)       // dark grey
      );
    }
    #pragma omp section
    {
      // Nereid: 0.0369 AU from Neptune, mass 0.0000000270 Earth, orbits Neptune in XZ plane
      // sits within Neptune Hill sphere (~116.0 sim units), outside Triton orbit
      float neptune_dist = 4510.5f;                            // 30.07 * 150
      float neptune_v    = sqrt(G * 332800.0f / neptune_dist);
      float nereid_dist  = 5.535f;                             // 0.0369 * 150
      float nereid_v     = sqrt(G * 17.15f / nereid_dist);    // orbital velocity around Neptune
      bodies[28] = createBody(
          glm::vec3(neptune_dist + nereid_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(neptune_v + nereid_v)),
          0.0000000270f,                        // mass in Earth units
          0.17f,                                // 0.0340 Earth radii * 5, floored for visibility
          glm::vec3(0.45f, 0.45f, 0.42f)       // grey
      );
    }
    #pragma omp section
    {
      // Larissa: 0.000494 AU from Neptune, mass 0.00000000618 Earth, orbits Neptune in XZ plane
      // sits within Neptune Hill sphere (~116.0 sim units), outside Neptune radius
      float neptune_dist = 4510.5f;                            // 30.07 * 150
      float neptune_v    = sqrt(G * 332800.0f / neptune_dist);
      float larissa_dist = 0.074f;                             // 0.000494 * 150
      float larissa_v    = sqrt(G * 17.15f / larissa_dist);   // orbital velocity around Neptune
      bodies[29] = createBody(
          glm::vec3(neptune_dist + larissa_dist, 0.0f, 0.0f),
          glm::vec3(0.0f, 0.0f, -(neptune_v + larissa_v)),
          0.00000000618f,                       // mass in Earth units
          0.15f,                                // 0.0291 Earth radii * 5, floored for visibility
          glm::vec3(0.4f, 0.4f, 0.38f)         // dark grey
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

  // Record trail positions — each thread owns its own body, no race condition
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

  // Draw ring if body has one
  if (body.hasRing) {
    glDisable(GL_LIGHTING);
    glColor4f(0.8f, 0.7f, 0.5f, 0.6f); // sandy semi-transparent ring color
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 360; i += 2) {
      float angle = glm::radians((float)i);
      glVertex3f(cos(angle) * body.ringInnerRadius, 0.0f, sin(angle) * body.ringInnerRadius);
      glVertex3f(cos(angle) * body.ringOuterRadius, 0.0f, sin(angle) * body.ringOuterRadius);
    }
    glEnd();
    glEnable(GL_LIGHTING);
  }

  glPopMatrix();
  gluDeleteQuadric(quad);
}


