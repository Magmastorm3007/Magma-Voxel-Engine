#pragma once

#include <glm/glm.hpp>

struct Projectile {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;  // Time before disappearing
};
