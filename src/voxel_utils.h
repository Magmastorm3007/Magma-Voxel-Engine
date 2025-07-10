// voxel_utils.h

#pragma once
#include <glm/glm.hpp>

inline glm::ivec3 toChunkPos(const glm::ivec3& voxelPos) {
    return voxelPos / 16;
}

inline glm::ivec3 toLocalPos(const glm::ivec3& voxelPos) {
    glm::ivec3 pos = voxelPos % 16;
    return glm::ivec3((pos.x + 16) % 16, (pos.y + 16) % 16, (pos.z + 16) % 16);
}
