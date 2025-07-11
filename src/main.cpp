#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "cube_renderer.h"
#include "camera.h"
#include "voxel_world.h"
#include "particle.h"

Camera camera(glm::vec3(0.0f, 30.0f, 30.0f));
VoxelWorld voxelWorld;
std::vector<Projectile> projectiles;

struct GunPart {
    glm::vec3 offset;
    glm::vec3 scale;
    glm::vec3 color;
};

std::vector<GunPart> gunParts = {
    { glm::vec3(0.0f, 0.0f, 0.0f),     glm::vec3(0.1f, 0.1f, 0.7f), glm::vec3(0.1f, 0.1f, 0.1f) },  // Barrel
    { glm::vec3(0.0f, -0.18f, 0.22f),  glm::vec3(0.13f, 0.25f, 0.13f), glm::vec3(0.15f, 0.08f, 0.02f) } // Handle
};

float lastX = 400, lastY = 300;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false;
    }

    float xOffset = float(xpos - lastX);
    float yOffset = float(lastY - ypos);
    lastX = float(xpos);
    lastY = float(ypos);

    camera.processMouseMovement(xOffset, yOffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glm::vec3 fireOrigin = camera.position + camera.front * 0.5f;
        glm::vec3 fireDirection = glm::normalize(camera.front);

        projectiles.push_back({ fireOrigin, fireDirection * 20.0f, 3.0f });
        projectiles.push_back({ fireOrigin, glm::vec3(0.0f), 0.05f });  // muzzle flash
    }
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime);
}

int main() {
    camera.front = glm::normalize(glm::vec3(0.0f, -0.5f, -1.0f));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Magma Voxel", nullptr, nullptr);
    if (!window) {
        std::cerr << "❌ Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "❌ Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); // Make sure gun parts are visible for testing
    glClearColor(0.52f, 0.80f, 0.92f, 1.0f);

    Shader shader("shaders/cube.vert", "shaders/cube.frag");
    CubeRenderer cubeRenderer;

    voxelWorld.generateTerrain(32, 32, 8);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
        glm::mat4 viewProj = projection * view;

        shader.use();
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setVec3("lightPos", glm::vec3(10.0f, 10.0f, 10.0f));
        shader.setVec3("viewPos", camera.position);

        // --- Voxels ---
        shader.setVec3("blockColor", glm::vec3(0.2f, 0.8f, 0.2f));
        voxelWorld.draw(cubeRenderer, shader, viewProj);

        // --- Projectiles ---
        for (auto& p : projectiles) {
            p.position += p.velocity * deltaTime;
            p.life -= deltaTime;

            if (glm::dot(p.velocity, p.velocity) > 0.0001f) {
                glm::ivec3 checkPos = glm::round(p.position);
                Voxel* voxel = voxelWorld.getVoxel(checkPos);
                if (voxel && voxel->active) {
                    voxelWorld.deactivateVoxel(checkPos);
                    p.life = 0.0f;
                }
            }

            glm::vec3 color = glm::length(p.velocity) < 0.01f
                ? glm::vec3(1.5f, 1.2f, 0.0f)
                : glm::vec3(1.0f, 0.2f, 0.2f);

            float size = glm::length(p.velocity) < 0.01f ? 0.15f : 0.2f;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), p.position);
            model = glm::scale(model, glm::vec3(size));
            shader.setMat4("model", glm::value_ptr(model));
            shader.setVec3("blockColor", color);
            cubeRenderer.draw(shader);
        }

        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                           [](const Projectile& p) { return p.life <= 0.0f; }),
            projectiles.end()
        );

        // --- Gun Rendering ---
        glm::mat4 camRot = glm::mat4(glm::mat3(camera.getViewMatrix()));
        glm::mat4 gunBase = glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, -0.3f, -1.5f)); // farther Z
        glm::mat4 gunTransform = camRot * gunBase;
        glm::mat4 gunVP = projection * gunTransform;

        for (const auto& part : gunParts) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), part.offset);
            model = glm::scale(model, part.scale);
            glm::mat4 finalModel = gunVP * model;

            shader.setMat4("model", glm::value_ptr(finalModel));
            shader.setVec3("blockColor", part.color);
            cubeRenderer.draw(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
