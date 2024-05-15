#ifndef KARTMODEL_H
#define KARTMODEL_H

#define GLM_FORCE_CTOR_INIT 
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

class KartModel {
public:
    KartModel();

    void handleInput(GLFWwindow* window, float deltaTime);
    void update(float deltaTime);

    glm::vec3 getPosition() const;

    glm::mat4 getModelMatrix() const;

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