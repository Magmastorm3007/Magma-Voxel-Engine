    #define STB_PERLIN_IMPLEMENTATION
    #include <stb_perlin.h>
    #include <algorithm>

    #include "voxel_world.h"
    #include "voxel_utils.h"
    #include <vector>
    #include <utility>

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

    void VoxelWorld::generateTerrain(int width, int depth, int maxHeight) {
        float scale = 0.1f; // Smaller = smoother terrain
        for (int x = -width / 2; x < width / 2; ++x) {
            for (int z = -depth / 2; z < depth / 2; ++z) {
                float noise = stb_perlin_noise3(x * scale, 0.0f, z * scale, 0, 0, 0);
                int height = static_cast<int>((noise + 1.0f) / 2.0f * maxHeight); // Normalize

                for (int y = 0; y <= height; ++y) {
                    voxels[{x, y, z}] = Voxel{};
                }
            }
        }
    }
    void VoxelWorld::deactivateVoxel(const glm::ivec3& worldPos) {
        auto it = voxels.find({worldPos.x, worldPos.y, worldPos.z});
        if (it != voxels.end() && it->second.active) {
            it->second.active = false;
        }
    }

    Voxel* VoxelWorld::getVoxel(const glm::ivec3& worldPos) {
        auto it = voxels.find({worldPos.x, worldPos.y, worldPos.z});
        if (it == voxels.end()) return nullptr;
        return &(it->second);
    }



    void VoxelWorld::draw(CubeRenderer& renderer, Shader& shader, const glm::mat4& viewProj) const {
        glm::vec3 cameraPos = glm::vec3(glm::inverse(viewProj)[3]); // Extract approximate camera position

        std::vector<std::pair<float, const VoxelPos*>> visibleVoxels;

        for (const auto& [pos, voxel] : voxels) {
            if (!voxel.active) continue;

            glm::vec3 worldPos = glm::vec3(pos.x, pos.y, pos.z);

            // Distance culling
            float distSq = glm::dot(worldPos - cameraPos, worldPos - cameraPos);
            if (distSq > 400.0f * 400.0f) continue;  // Skip far voxels (e.g. >200 units)

            // Skip fully surrounded blocks (face culling)
            VoxelPos neighbors[6] = {
                {pos.x + 1, pos.y, pos.z}, {pos.x - 1, pos.y, pos.z},
                {pos.x, pos.y + 1, pos.z}, {pos.x, pos.y - 1, pos.z},
                {pos.x, pos.y, pos.z + 1}, {pos.x, pos.y, pos.z - 1}
            };

            bool hasVisibleFace = false;
            for (const auto& n : neighbors) {
                auto it = voxels.find(n);
                if (it == voxels.end() || !it->second.active) {
                    hasVisibleFace = true;
                    break;
                }
            }
            if (!hasVisibleFace) continue;

            // Frustum culling
            if (!isCubeInFrustum(worldPos, viewProj)) continue;

            visibleVoxels.emplace_back(distSq, &pos);
        }

        // Sort for transparency/farther-to-closer (optional)
        std::sort(visibleVoxels.begin(), visibleVoxels.end(),
                [](const auto& a, const auto& b) { return a.first > b.first; });

        for (const auto& [_, posPtr] : visibleVoxels) {
            glm::vec3 worldPos = glm::vec3(posPtr->x, posPtr->y, posPtr->z);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
            shader.setMat4("model", &model[0][0]);
            renderer.draw(shader);
        }
    }

    // Note: The shader should have uniform variables for model, view, projection matrices,