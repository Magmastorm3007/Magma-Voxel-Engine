# 🟫 Magma Voxel Engine

OpenGL voxel sandbox. FPS camera, block collision, terrain via Perlin noise. Shoot voxels, render gun, keep it light.

---

## 🎮 Preview

![screenshot](pics/main_view.png)


---

## 🔧 Features

- Procedural terrain (Perlin noise)
- WASD + mouse camera
- Gun rendering in screen space
- Projectile system with voxel collision
- Distance + frustum + face culling for perf
- Minimal design, extendable

---

## 🗂️ Folder Structure

├── shaders/
│ ├── cube.vert
│ └── cube.frag
├── pics/
│ ├── main_view.png
│ └── gun_view.png
├── src/
│ ├── main.cpp
│ ├── camera.h
│ ├── cube_renderer.h
│ ├── voxel_world.h
│ ├── voxel_utils.h
│ ├── shader.h
│ └── particle.h
└── README.md


---

## ⚙️ Architecture

### 🚀 main.cpp
- Initializes OpenGL, sets up window and input
- Handles update + render loop
- Gun drawn in screen space (no camera translation)

### 🧱 VoxelWorld
- Stores voxels in `unordered_map<ivec3, Voxel>`
- Generates terrain using Perlin noise
- Only draws visible, nearby blocks

### 🧊 CubeRenderer
- Renders cubes using a single VAO
- Uses model/view/projection shader uniforms



### 💥 Projectile
- Spawned from camera front
- Destroys voxel on impact
- Uses manual collision check with rounded voxel positions

---

## 🎮 Controls

| Action      | Key         |
|-------------|-------------|
| Move        | W / A / S / D |
| Look        | Mouse       |
| Shoot       | Left Click  |
| Exit        | ESC         |

---

## 🛠️ Build

**Dependencies**: GLFW, GLAD, GLM, stb

```bash
git clone <repo>
mkdir build && cd build
cmake ..
make
./MagmaVoxel

Notes

    Terrain is scaled noise: stb_perlin_noise3(...)

    Voxel deletion = setting .active = false

    Camera follows standard FPS style (yaw-pitch)

    Gun doesn't move with player position, only rotation

    Cull:

        Far voxels (distSq > threshold)

        Fully surrounded blocks

        Outside frustum via bounding check

📄 License

MIT — use it however, build wild stuff.