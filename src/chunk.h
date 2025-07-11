#pragma once

#include "camera.h"
#include "voxel_world.h"
#include "particle.h"
#include <vector>
#include <GLFW/glfw3.h>

// ✅ Declare global variables here (don't define them!)
extern Camera camera;
extern VoxelWorld voxelWorld;
extern std::vector<Projectile> projectiles;

extern float lastX, lastY;
extern bool firstMouse;
extern float deltaTime, lastFrame;

// ✅ Declare callbacks (they’re defined in main.cpp)
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
