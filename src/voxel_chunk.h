#pragma once

#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include "voxel_utils.h"
#include "shader.h" // 🔧 Add this in voxel_chunk.cpp
#include <glm/gtc/type_ptr.hpp>
#include<voxel_utils.h>


class Shader;
class CubeRenderer;

constexpr int CHUNK_SIZE = 16;

enum class FaceDirection {
    Right = 0,
    Left,
    Top,
    Bottom,
    Front,
    Back
};

struct Voxel {
    bool active = false;
};

class VoxelChunk {
public:
    VoxelChunk();

    void generateTerrainChunk(const glm::ivec3& chunkPos, int maxHeight);
    void updateMesh();
    void uploadMesh();
    void draw(CubeRenderer& renderer, Shader& shader, const glm::mat4& viewProj);
    Voxel* getVoxel(int x, int y, int z);
    bool dirty = true;

private:
    Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    std::vector<float> meshData;
    GLuint VAO = 0, VBO = 0;

    bool isVoxelSolid(int x, int y, int z) const;
    void appendFace(FaceDirection dir, const glm::vec3& pos);
};
