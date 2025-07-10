#include "voxel_chunk.h"
#include "shader.h"
#include "cube_renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include "voxel_utils.h"

VoxelChunk::VoxelChunk() {
    dirty = true;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    for (int x = 0; x < CHUNK_SIZE; ++x)
        for (int y = 0; y < CHUNK_SIZE; ++y)
            for (int z = 0; z < CHUNK_SIZE; ++z)
                voxels[x][y][z].active = false;
}

void VoxelChunk::generateTerrainChunk(const glm::ivec3& chunkPos, int maxHeight) {
    for (int x = 0; x < CHUNK_SIZE; ++x)
        for (int y = 0; y < CHUNK_SIZE; ++y)
            for (int z = 0; z < CHUNK_SIZE; ++z)
                voxels[x][y][z].active = y < maxHeight / 2;

    dirty = true;
}
void VoxelChunk::appendFace(FaceDirection dir, const glm::vec3& pos) {
    static const std::vector<float> faceVertices[6] = {
        // Right
        { 0.5f,-0.5f,-0.5f, 1,0,0,  0.5f,-0.5f, 0.5f, 1,0,0,  0.5f, 0.5f, 0.5f, 1,0,0,
          0.5f, 0.5f, 0.5f, 1,0,0,  0.5f, 0.5f,-0.5f, 1,0,0,  0.5f,-0.5f,-0.5f, 1,0,0 },
        // Left
        { -0.5f,-0.5f, 0.5f,-1,0,0, -0.5f,-0.5f,-0.5f,-1,0,0, -0.5f, 0.5f,-0.5f,-1,0,0,
         -0.5f, 0.5f,-0.5f,-1,0,0, -0.5f, 0.5f, 0.5f,-1,0,0, -0.5f,-0.5f, 0.5f,-1,0,0 },
        // Top
        { -0.5f, 0.5f,-0.5f, 0,1,0,  0.5f, 0.5f,-0.5f, 0,1,0,  0.5f, 0.5f, 0.5f, 0,1,0,
          0.5f, 0.5f, 0.5f, 0,1,0, -0.5f, 0.5f, 0.5f, 0,1,0, -0.5f, 0.5f,-0.5f, 0,1,0 },
        // Bottom
        { -0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f, 0.5f, 0,-1,0,
          0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f,-0.5f, 0,-1,0 },
        // Front
        { -0.5f,-0.5f, 0.5f, 0,0,1,  0.5f,-0.5f, 0.5f, 0,0,1,  0.5f, 0.5f, 0.5f, 0,0,1,
          0.5f, 0.5f, 0.5f, 0,0,1, -0.5f, 0.5f, 0.5f, 0,0,1, -0.5f,-0.5f, 0.5f, 0,0,1 },
        // Back
        { -0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f, 0.5f,-0.5f, 0,0,-1,
          0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f,-0.5f,-0.5f, 0,0,-1 }
    };

    const auto& verts = faceVertices[int(dir)];
    for (size_t i = 0; i < verts.size(); i += 6) {
        meshData.push_back(verts[i + 0] + pos.x);
        meshData.push_back(verts[i + 1] + pos.y);
        meshData.push_back(verts[i + 2] + pos.z);
        meshData.push_back(verts[i + 3]);
        meshData.push_back(verts[i + 4]);
        meshData.push_back(verts[i + 5]);
    }
}

            

 
bool VoxelChunk::isVoxelSolid(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE)
        return false;
    return voxels[x][y][z].active;
}

void VoxelChunk::updateMesh() {
    meshData.clear();

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                if (!voxels[x][y][z].active) continue;
                glm::vec3 pos = glm::vec3(x, y, z);
                if (!isVoxelSolid(x + 1, y, z)) appendFace(FaceDirection::Right, pos);
                if (!isVoxelSolid(x - 1, y, z)) appendFace(FaceDirection::Left, pos);
                if (!isVoxelSolid(x, y + 1, z)) appendFace(FaceDirection::Top, pos);
                if (!isVoxelSolid(x, y - 1, z)) appendFace(FaceDirection::Bottom, pos);
                if (!isVoxelSolid(x, y, z + 1)) appendFace(FaceDirection::Front, pos);
                if (!isVoxelSolid(x, y, z - 1)) appendFace(FaceDirection::Back, pos);
            }
        }
    }

    uploadMesh();
    dirty = false;
}

void VoxelChunk::uploadMesh() {
    if (meshData.empty()) return;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);

    // layout(location = 0) -> vec3 position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // layout(location = 1) -> vec3 normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void VoxelChunk::draw(CubeRenderer& renderer, Shader& shader, const glm::mat4& viewProj) {
    glm::mat4 model(1.0f);
    shader.setMat4("model", glm::value_ptr(model));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, meshData.size() / 6); // 3 for position + 3 for normal
    glBindVertexArray(0);
}

Voxel* VoxelChunk::getVoxel(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE)
        return nullptr;
    return &voxels[x][y][z];
}
