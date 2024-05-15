#ifndef KARTMODEL_H
#define KARTMODEL_H

#include <glm.hpp>
#include <glfw3.h>

class KartModel {
public:
    KartModel();

    void handleInput(GLFWwindow* window, float deltaTime);
    void update(float deltaTime);

    glm::vec3 getPosition() const;

    glm::vec3 getDirection() const;

private:
    glm::vec3 position;
    glm::vec3 velocity;
    float direction;
    float speed;
    float turnSpeed;
    glm::vec3 acceleration;
    float brakeForce;
};

#endif // KARTMODEL_H