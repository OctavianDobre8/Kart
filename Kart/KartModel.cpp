#include "KartModel.h"
#include <cmath>

KartModel:: KartModel() : position(3.9f, -0.46f, -1.0f), velocity(0.0f, 0.0f, 0.0f), acceleration(0.0f, 0.0f, 0.0f), direction(0.0f), speed(10.0f), brakeForce(20.0f), turnSpeed(0.01f) {}

void KartModel::handleInput(GLFWwindow* window, float deltaTime) {
    float radianDirection = glm::radians(direction);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        acceleration += glm::vec3(sin(radianDirection), 0.0f, cos(radianDirection)) * speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        acceleration -= glm::vec3(sin(radianDirection), 0.0f, cos(radianDirection)) * brakeForce * deltaTime; // Use brakeForce when 'S' is pressed
    }
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        direction -= turnSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        direction += turnSpeed * deltaTime;
    }
}

void KartModel::update(float deltaTime) {
    velocity += acceleration * deltaTime; // Update velocity based on acceleration
    position += velocity * deltaTime;
    // Add some friction to gradually slow down the kart
    velocity *= 0.99f;
    position.y = -0.43f;
}


glm::vec3 KartModel::getPosition() const {
    return position;
}