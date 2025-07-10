// cube_renderer.h
#ifndef CUBE_RENDERER_H
#define CUBE_RENDERER_H

#include <glad/gl.h>
#include "shader.h"

class CubeRenderer {
public:
    CubeRenderer();
    ~CubeRenderer();
    void draw(const Shader& shader) const;

private:
    unsigned int VAO, VBO;
    void setupCube();
};

#endif
