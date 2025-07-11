#include "chunk.h"
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>


#include <stb_perlin.h>
// Static face data and directions
namespace {
    constexpr glm::ivec3 offsets[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };

    constexpr glm::vec3 normals[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };

    constexpr glm::vec3 faces[6][6] = {
        // Right (+X)
        {
            {0.5f, -0.5f, -0.5f}, {0.5f,  0.5f, -0.5f}, {0.5f,  0.5f,  0.5f},
            {0.5f, -0.5f, -0.5f}, {0.5f,  0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}
        },
        // Left (-X)
        {
            {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f},
            {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}
        },
        // Top (+Y)
        {
            {-0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f},
            {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f, -0.5f}
        },
        // Bottom (-Y)
        {
            {-0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f},
            {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f,  0.5f}
        },
        // Front (+Z)
        {
            {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f},
            {-0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}
        },
        // Back (-Z)
        {
            { 0.5f, -0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f},
            { 0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}
        }
    };
}

VoxelChunk::VoxelChunk() {
    for (int x = 0; x < CHUNK_SIZE; ++x)
        for (int y = 0; y < CHUNK_SIZE; ++y)
            for (int z = 0; z < CHUNK_SIZE; ++z)
                voxels[x][y][z].active = false;
}

Voxel* VoxelChunk::getVoxel(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
        return nullptr;
    return &voxels[x][y][z];
}

bool VoxelChunk::isVoxelSolid(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
        return false;
    return voxels[x][y][z].active;
}

void VoxelChunk::generateTerrainChunk(const glm::ivec3& chunkPos, int maxHeight) {
    for (int x = 0; x < CHUNK_SIZE; ++x)
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            int worldX = chunkPos.x * CHUNK_SIZE + x;
            int worldZ = chunkPos.z * CHUNK_SIZE + z;

            float noise = stb_perlin_noise3(worldX * 0.05f, 0.0f, worldZ * 0.05f, 0, 0, 0);
            int height = static_cast<int>((noise + 1.0f) * 0.5f * maxHeight);

            for (int y = 0; y <= height && y < CHUNK_SIZE; ++y) {
                voxels[x][y][z].active = true;
            }
        }
    dirty = true;
}

void VoxelChunk::updateMesh() {
    meshData.clear();

    for (int x = 0; x < CHUNK_SIZE; ++x)
        for (int y = 0; y < CHUNK_SIZE; ++y)
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                if (!voxels[x][y][z].active) continue;

                for (int i = 0; i < 6; ++i) {
                    glm::ivec3 adj = glm::ivec3(x, y, z) + offsets[i];
                    if (!isVoxelSolid(adj.x, adj.y, adj.z))
                        appendFace(static_cast<FaceDirection>(i), glm::vec3(x, y, z));
                }
            }

    uploadMesh();
    dirty = false;
}

void VoxelChunk::uploadMesh() {
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void VoxelChunk::appendFace(FaceDirection dir, const glm::vec3& pos) {
    const auto& face = faces[static_cast<int>(dir)];
    glm::vec3 normal = normals[static_cast<int>(dir)];

    for (const auto& vertex : face) {
        glm::vec3 finalPos = pos + vertex;
        meshData.insert(meshData.end(), { finalPos.x, finalPos.y, finalPos.z,
                                          normal.x, normal.y, normal.z });
    }
}

void VoxelChunk::draw(CubeRenderer& renderer, Shader& shader, const glm::mat4& viewProj) {
    if (meshData.empty()) return;

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", glm::value_ptr(model));
    shader.setVec3("blockColor", glm::vec3(0.4f, 0.3f, 0.2f)); // ⛰️ Dirt color

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(meshData.size() / 6));
    glBindVertexArray(0);
}
