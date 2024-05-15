#include "KartModel.h"
#include <cmath>


KartModel::KartModel()
    : position(3.9f, -0.46f, -1.0f),
    velocity(0.0f, 0.0f, 0.0f),
    acceleration(0.0f, 0.0f, 0.0f),
    direction(0.0f),
    speed(35.0f),
    brakeForce(40.0f),
    turnSpeed(150.0f) {} // Adjusted turn speed for more responsive turning

void KartModel::handleInput(GLFWwindow* window, float deltaTime) {
    float radianDirection = glm::radians(direction);
    glm::vec3 forwardDirection = glm::vec3(sin(radianDirection), 0.0f, cos(radianDirection));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        acceleration = forwardDirection * speed;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        acceleration = -forwardDirection * brakeForce;
    }
    else {
        acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        direction -= turnSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        direction += turnSpeed * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        position = glm::vec3(3.9f, -0.46f, -1.0f);
        velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
        direction = 0.0f;
    }
}

void KartModel::update(float deltaTime) {
    velocity += acceleration * deltaTime;
    glm::vec3 newPosition = position + velocity * deltaTime;

    // Ensure the kart stays within bounds
    if (newPosition.x >= -5.0f && newPosition.x <= 5.0f && newPosition.z >= -5.0f && newPosition.z <= 5.0f) {
        position = newPosition;
    }

    // Apply friction
    velocity *= 0.99f;

    // Maintain the kart at a constant height
    position.y = -0.43f;
}

glm::vec3 KartModel::getPosition() const {
    return position;
}

glm::mat4 KartModel::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(direction), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.001f)); // Adjust scale as needed
    return model;
}

float KartModel::getDirection() const {
    return direction;
}
