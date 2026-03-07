# **Gravity-Based Solar System Simulation (OpenGL + OpenMP)**

### **Project Summary**

This C++ program implements a real-time **gravitational N-body simulation** of the solar system with all 8 planets and 22 major moons (30 bodies total). Using **OpenGL/GLFW** for rendering and **OpenMP** for parallel physics computation, the simulation runs gravitational force integration at 10 physics steps per rendered frame and renders each body as a lit sphere with a fading orbit trail. The camera supports zoom-to-cursor with smooth panning across the full solar system scale.

---

### **Core Features**

- 30 gravitationally interacting bodies — Sun, all 8 planets, 22 major moons.
- Real-time **N-body gravity** via symplectic Euler integration with OpenMP parallelization.
- **Orbit trails** with per-vertex alpha fade over the last 200 recorded positions.
- **Saturn's rings** rendered as a semi-transparent triangle strip.
- **Zoom-to-cursor** — multiplicative scroll zoom that pans toward the world point under the cursor.
- **Minimum apparent size** enforcement — moons are always at least 2px on screen regardless of zoom level, matching the approach used by SpaceEngine and Universe Sandbox.
- Dynamic near/far clip planes that scale with camera distance, preserving depth buffer precision at all zoom levels.

---

### **Scaling — What Is Accurate and What Is Not**

This is the most important section to understand before interpreting the simulation visually.

#### **Distance Scale**

Orbital distances are mapped linearly: **1 AU = 150 simulation units**. No compression or logarithmic scaling is applied. Physics positions are used directly for rendering. This means:

| Body | Real Distance | Sim Distance |
| :--- | :--- | :--- |
| Mercury | 0.387 AU | 58.05 units |
| Venus | 0.723 AU | 108.45 units |
| Earth | 1.000 AU | 150.00 units |
| Mars | 1.524 AU | 228.60 units |
| Jupiter | 5.203 AU | 780.45 units |
| Saturn | 9.537 AU | 1430.55 units |
| Uranus | 19.19 AU | 2878.50 units |
| Neptune | 30.07 AU | 4510.50 units |

**Orbital distances are accurate relative to each other.** The ratio between any two orbital radii matches reality.

#### **Radius Scale**

Planet radii use **Earth = 5.0 sim units** as an anchor, with all other bodies scaled proportionally by their real Earth-radii ratio:
```
renderRadius = (realRadius / earthRadius) * 5.0
```

| Body | Real Radius (Earth = 1) | Sim Radius |
| :--- | :--- | :--- |
| Sun | 109.0 | 32.8 (intentionally compressed — see below) |
| Earth | 1.000 | 5.00 |
| Jupiter | 11.21 | 56.05 |
| Saturn | 9.45 | 47.25 |
| Uranus | 4.01 | 20.05 |
| Neptune | 3.89 | 19.45 |

**The Sun's radius is the only intentional inaccuracy.** At true scale the Sun would be 545 sim units across — larger than Mercury's entire orbit at 58 sim units — making the inner solar system completely obscured. It is compressed to 32.8 units. All planet and moon radii are proportionally accurate to each other.

#### **The Scale Mismatch Problem**

The fundamental visual tension in any solar system renderer: distances and radii cannot both be true-scale simultaneously. At true scale, Earth (5.0 sim units) orbits at 150 sim units — a radius-to-distance ratio of 1:30. Every real planet would be invisible at full zoom. This is not a bug — it is an unavoidable property of the solar system's actual geometry.

**Solution: minimum apparent size.** In `drawBody`, a floor is applied to the rendered radius so no body subtends fewer than 2 pixels on screen:
```cpp
float minRadius = distToCam * 0.00138f; // 2px on 600px at 45° FOV
float renderRadius = fmaxf(body.radius, minRadius);
```

The constant `0.00138` is derived from the screen-space angular size of 1 pixel:
```
pixelAngle = tan(FOV/2) / (screenHeight/2) = tan(22.5°) / 300 ≈ 0.00138
```

#### **Moon Orbit Distances**

Moon orbital distances face the same scale mismatch. At true scale the Moon orbits at 0.00257 AU = 0.385 sim units — well inside Earth's rendered sphere of 5.0 radius. All moon orbital distances are compressed to sit just outside their parent planet's rendered sphere while remaining inside their Hill sphere:

| Moon | True Orbit (sim) | Simulated Orbit (sim) | Hill Sphere (sim) |
| :--- | :--- | :--- | :--- |
| Moon | 0.385 | 1.2 | ~1.5 |
| Phobos | 0.009 | 0.3 | ~1.086 |
| Io | 0.063 | 0.423 | ~53.3 |
| Titan | 0.226 | 1.226 | ~65.4 |

Moon orbital distances are **not accurate in absolute terms** but are **physically stable** — each sits within its parent's Hill sphere and outside the planet's rendered radius, so gravity produces correct circular orbits.

---

### **Physics**

#### **Gravitational Force**

All 30 bodies interact gravitationally every step. Force on body `i` from body `j`:
```
F_ij = G * m_i * m_j / (|r_ij|² + ε) * r̂_ij
```

Where `ε = 1e-4` is a softening term preventing singularities at close approach. The gravitational constant `G = 6.67430e-3` is scaled down from the SI value to match the simulation's unit system (distances in sim units, mass in Earth masses).

#### **Integration**

Symplectic Euler (semi-implicit Euler) integration:
```
a_i = F_i / m_i
v_i += a_i * dt
x_i += v_i * dt
```

`dt = 0.001`, applied 10 times per rendered frame (10 substeps). This gives effective physics at `dt = 0.01` equivalent while keeping each individual step small enough for stable moon orbits. Reducing substep size from `dt = 0.01` to `dt = 0.001` was the fix that stabilized close moon orbits (Phobos, Deimos, inner Galilean moons).

#### **Initial Velocities**

All bodies start on circular orbits. Orbital velocity for a body at distance `r` from its parent of mass `M`:
```
v = sqrt(G * M / r)
```

Planets orbit the Sun. Moons orbit their parent planet — their initial velocity is the planet's orbital velocity plus the moon's local circular velocity:
```
v_moon = v_planet + sqrt(G * M_planet / r_moon)
```

#### **Parallelization**

Three separate OpenMP parallel loops per physics step:

1. Force accumulation — `O(n²)`, parallelized over `i`, each thread accumulates into a private `totalForce`.
2. Velocity and position update — `O(n)`, embarrassingly parallel.
3. Trail recording — `O(n)`, each thread writes only to its own body's deque, no race condition.

Trail update is a separate loop from position update because `std::deque` reallocation during push_back is not thread-safe if mixed with reads from another loop.

---

### **Rendering**

#### **Moon Visibility**

Moons are rendered with `GL_DEPTH_TEST` disabled when `body.radius < 5.0`. Without this, a moon sitting inside its planet's rendered sphere (which it always does at solar system scale) is occluded by the planet's geometry in the depth buffer. Disabling depth test for small bodies ensures they always draw on top. This is the same technique used by Universe Sandbox and SpaceEngine.

#### **Lighting**

Fixed-function OpenGL pipeline with `GL_LIGHT0` positioned at the Sun (origin) each frame after `gluLookAt`. The Sun uses `GL_EMISSION` so it renders at full brightness regardless of the lighting calculation. Planets use `GL_AMBIENT_AND_DIFFUSE` driven by `glColor3f`.

#### **Zoom-to-Cursor**

On scroll, the cursor position is unprojected from screen space to world space using `gluUnProject` on near and far planes, then intersected with the orbital plane `Y=0` to get a world point. Camera target shifts toward that world point proportional to the zoom ratio:
```
camTarget = worldPoint + (camTarget - worldPoint) * (newZoom / oldZoom)
```

---

### **Accuracy Summary**

| Property | Accurate? | Notes |
| :--- | :--- | :--- |
| Relative orbital distances | ✅ Yes | Linear scale, 1 AU = 150 units |
| Relative planet radii | ✅ Yes | Earth = 5.0 anchor, all others proportional |
| Sun radius | ❌ Compressed | True scale occludes inner solar system |
| Moon orbital distances | ❌ Compressed | Physically stable, not true scale |
| Orbital velocities | ✅ Yes | Derived from circular orbit formula |
| Gravitational interactions | ✅ Yes | Full N-body, all 30 bodies interact |
| Moon masses | ✅ Yes | Real Earth-mass ratios |
| Orbital periods | ✅ Approximate | Emerge from physics, not hardcoded |

---

### **File Overview**

| File | Description |
| :--- | :--- |
| **main.cpp** | GLFW window, OpenGL setup, camera, zoom-to-cursor scroll callback, render loop. |
| **functions.h** | `Body` struct, constants (`G`, `TRAIL_LENGTH`), extern globals, function declarations. |
| **functions.cpp** | `createSolarSystem`, `updateGravity`, `drawBody` — all physics and rendering logic. |
| **Makefile** | Compiles with `-fopenmp`, links `glfw`, `GL`, `GLU`, `glut`. |

---

### **How to Compile and Run**

#### **1. Install Dependencies**

- **Arch Linux**:
```bash
    sudo pacman -S glfw-x11 mesa glu freeglut glm gcc make
```

- **Ubuntu / Debian**:
```bash
    sudo apt install libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev libglm-dev g++ make
```

- **Fedora**:
```bash
    sudo dnf install glfw-devel mesa-libGL-devel mesa-libGLU-devel freeglut-devel glm-devel gcc-c++ make
```

#### **2. Compile**
```bash
make
```

#### **3. Run**
```bash
./solar_system
```

#### **4. Controls**

- **Scroll up** — zoom in toward cursor position
- **Scroll down** — zoom out toward full solar system view

---

### **Known Limitations**

- Orbits are coplanar — all bodies orbit in the XZ plane. Real inclinations are not modeled.
- No axial tilt or rotation of bodies.
- Symplectic Euler accumulates small energy errors over very long runs — a higher-order integrator (Verlet, RK4) would improve long-term stability.
- Fixed timestep — simulation speed is tied to frame rate.
