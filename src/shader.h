#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glad/gl.h>
#include <glm/glm.hpp> // 🆕 Required for glm::vec3

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    
    void use() const;
    
    void setMat4(const std::string &name, const float* value) const;
    
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const; // 🆕 ADDED

    void setFloat(const std::string &name, float value) const;
    void setInt(const std::string &name, int value) const;
};

#endif
