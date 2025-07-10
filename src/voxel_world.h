#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include <glm/glm.hpp>
#include <unordered_map>
#include "cube_renderer.h"
#include "shader.h"

struct Voxel {
    bool active = true;
    // You can later add: material, type, etc.
};

struct VoxelPos {
    int x, y, z;

    bool operator==(const VoxelPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct VoxelHash {
    std::size_t operator()(const VoxelPos& pos) const {
        return std::hash<int>()(pos.x) ^ std::hash<int>()(pos.y << 1) ^ std::hash<int>()(pos.z << 2);
    }
};

class VoxelWorld {
public:
    std::unordered_map<VoxelPos, Voxel, VoxelHash> voxels;

    void generateFlatGround(int width, int depth);
   void draw(CubeRenderer& renderer, Shader& shader, const glm::mat4& viewProj) const;
   void generateTerrain(int width, int depth, int maxHeight);
 
    // You can add more methods for generating different terrains, adding/removing voxels, etc.
};

#endif
