#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    float yaw;
    float pitch;

    float speed;
    float sensitivity;

    Camera(glm::vec3 startPos);

    glm::mat4 getViewMatrix() const;
    void processMouseMovement(float xOffset, float yOffset);
    void processKeyboard(int direction, float deltaTime);
};

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

#endif
