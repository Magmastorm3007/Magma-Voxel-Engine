#include "voxel_world.h"
#include "cube_renderer.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// --- Frustum culling helper ---
bool isCubeInFrustum(const glm::vec3& pos, const glm::mat4& VP) {
    const float half = 0.5f;
    glm::vec3 offsets[] = {
        {-half, -half, -half}, { half, -half, -half},
        {-half,  half, -half}, { half,  half, -half},
        {-half, -half,  half}, { half, -half,  half},
        {-half,  half,  half}, { half,  half,  half}
    };

    for (const auto& offset : offsets) {
        glm::vec4 clip = VP * glm::vec4(pos + offset, 1.0f);
        if (clip.w == 0.0f) continue;
        clip /= clip.w;

        if (clip.x >= -1 && clip.x <= 1 &&
            clip.y >= -1 && clip.y <= 1 &&
            clip.z >= -1 && clip.z <= 1) {
            return true;
        }
    }

    return false;
}

void VoxelWorld::generateFlatGround(int width, int depth) {
    for (int x = -width / 2; x < width / 2; ++x) {
        for (int z = -depth / 2; z < depth / 2; ++z) {
            voxels[{x, 0, z}] = Voxel{};
        }
    }
}

void VoxelWorld::draw(CubeRenderer& renderer, Shader& shader, const glm::mat4& viewProj) const {
    for (const auto& [pos, voxel] : voxels) {
        if (!voxel.active) continue;
        glm::vec3 worldPos = glm::vec3(pos.x, pos.y, pos.z);

        // Face culling — skip cubes that are fully surrounded
        VoxelPos neighbors[6] = {
            {pos.x + 1, pos.y, pos.z},
            {pos.x - 1, pos.y, pos.z},
            {pos.x, pos.y + 1, pos.z},
            {pos.x, pos.y - 1, pos.z},
            {pos.x, pos.y, pos.z + 1},
            {pos.x, pos.y, pos.z - 1}
        };

        bool hasVisibleFace = false;
        for (const auto& n : neighbors) {
            auto it = voxels.find(n);
            if (it == voxels.end() || !it->second.active) {
                hasVisibleFace = true;
                break;
            }
        }

        if (!isCubeInFrustum(worldPos, viewProj)) continue;

        // Frustum culling — skip cubes not in view
        
        if (!isCubeInFrustum(worldPos, viewProj)) continue;

        // Draw visible voxel
        glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
        shader.setMat4("model", &model[0][0]);
        renderer.draw(shader);
    }
}
// Note: The shader should have uniform variables for model, view, projection matrices,