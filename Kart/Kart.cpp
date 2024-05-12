#include <stdlib.h> 
#include <stdio.h>
#include <math.h> 
#include <vector>

#include <Windows.h>
#include <locale>
#include <codecvt>

#include <GL/glew.h>

#define GLM_FORCE_CTOR_INIT 
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <stb_image.h>
#include "Camera.h"
#include "Shader.h"
#include "Model.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera* pCamera = nullptr;

void Cleanup()
{
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

unsigned int CreateTexture(const std::string& strTexturePath);

float scale = 0.001f;


float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,     
	 1.0f,  1.0f, -1.0f,    
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};


int main(int argc, char** argv)
{
	std::string strFullExeFileName = argv[0];
	std::string strExePath;
	const size_t last_slash_idx = strFullExeFileName.rfind('\\');
	if (std::string::npos != last_slash_idx) {
		strExePath = strFullExeFileName.substr(0, last_slash_idx);
	}


	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Karting Simulator", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Floor vertices
	float floorVertices[] = {
		// positions          // texture Coords 
		5.0f, -0.5f,  5.0f,  1.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f,

		5.0f, -0.5f,  5.0f,  1.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f,
		5.0f, -0.5f, -5.0f,  1.0f, 1.0f
	};

	// Floor VAO si VBO
	unsigned int floorVAO, floorVBO;
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), &floorVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// Textures
	unsigned int floorTexture = CreateTexture(strExePath + "\\Map.jpg");
	unsigned int kartTexture = CreateTexture(strExePath + "\\Kart.jpg");
	unsigned int treeTexture = CreateTexture(strExePath + "\\Tree.PNG");

	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 0.0, 3.0));

	glm::vec3 lightPos(0.0f, 3.0f, 1.0f);

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);

	Shader  lightingShader("PhongLight.vs", "PhongLight.fs");
	Shader shaderFloor("Floor.vs", "Floor.fs");
	Shader shaderBlending("Blending.vs", "Blending.fs");
	shaderBlending.SetInt("texture1", 0);

	std::string kartObjFileName = (currentPath + "\\resources\\models\\Kart\\model.obj");
	Model kartObjModel(kartObjFileName, false);

	std::string TreeObjFileName = (currentPath + "\\resources\\models\\Tree\\Tree.obj");
	Model treeObjModel(TreeObjFileName, false);

	std::string BushObjFileName = (currentPath + "\\resources\\models\\Bush\\skSpikyPlantMesh.obj");
	Model bushObjModel(BushObjFileName, false);

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0);

		shaderFloor.Use();
		glm::mat4 projection = pCamera->GetProjectionMatrix();
		glm::mat4 view = pCamera->GetViewMatrix();
		shaderFloor.SetMat4("projection", projection);
		shaderFloor.SetMat4("view", view);

		// Draw floor
		glBindVertexArray(floorVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);

		shaderFloor.SetMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		lightPos.x = 0;
		lightPos.z = 0;

		// Kart position
		glm::mat4 kartModel = glm::mat4(1.0f);
		kartModel = glm::translate(kartModel, glm::vec3(3.9f, -0.46f, -1.0f));
		kartModel = glm::scale(kartModel, glm::vec3(scale));

		// Kart Texture
		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);
		shaderBlending.SetMat4("model", kartModel);
		glBindTexture(GL_TEXTURE_2D, kartTexture);

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", kartModel);
		kartObjModel.Draw(lightingShader);

		// Tree1 position
		glm::mat4 treeModel1 = glm::mat4(1.0f);
		treeModel1 = glm::translate(treeModel1, glm::vec3(3.0f, -0.46f, -1.0f));
		treeModel1 = glm::scale(treeModel1, glm::vec3(0.005f));

		// Tree1 Texture
		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);
		shaderBlending.SetMat4("model", treeModel1);
		glBindTexture(GL_TEXTURE_2D, treeTexture);

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", treeModel1);
		treeObjModel.Draw(lightingShader);

		// Tree2 position
		glm::mat4 treeModel2 = glm::mat4(1.0f);
		treeModel2 = glm::translate(treeModel2, glm::vec3(-0.5f, -0.46f, -2.0f));
		treeModel2 = glm::scale(treeModel2, glm::vec3(0.005f));

		// Tree2 Texture
		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);
		shaderBlending.SetMat4("model", treeModel2);
		glBindTexture(GL_TEXTURE_2D, treeTexture);

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", treeModel2);
		treeObjModel.Draw(lightingShader);

		// Tree3 position
		glm::mat4 treeModel3 = glm::mat4(1.0f);
		treeModel3 = glm::translate(treeModel3, glm::vec3(4.5f, -0.46f, 4.5f));
		treeModel3 = glm::scale(treeModel3, glm::vec3(0.005f));

		// Tree3 Texture
		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);
		shaderBlending.SetMat4("model", treeModel3);
		glBindTexture(GL_TEXTURE_2D, treeTexture);

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", treeModel3);
		treeObjModel.Draw(lightingShader);

		// Tree4 position
		glm::mat4 treeModel4 = glm::mat4(1.0f);
		treeModel4 = glm::translate(treeModel4, glm::vec3(4.5f, -0.46f, -4.5f));
		treeModel4 = glm::scale(treeModel4, glm::vec3(0.005f));

		// Tree4 Texture
		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);
		shaderBlending.SetMat4("model", treeModel4);
		glBindTexture(GL_TEXTURE_2D, treeTexture);

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", treeModel4);
		treeObjModel.Draw(lightingShader);

		// Tree5 position
		glm::mat4 treeModel5 = glm::mat4(1.0f);
		treeModel5 = glm::translate(treeModel5, glm::vec3(-3.5f, -0.46f, 4.2f));
		treeModel5 = glm::scale(treeModel5, glm::vec3(0.005f));

		// Tree5 Texture
		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);
		shaderBlending.SetMat4("model", treeModel5);
		glBindTexture(GL_TEXTURE_2D, treeTexture);

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", treeModel5);
		treeObjModel.Draw(lightingShader);

		// Tree6 position
		glm::mat4 treeModel6 = glm::mat4(1.0f);
		treeModel6 = glm::translate(treeModel6, glm::vec3(-4.2f, -0.46f, 3.0f));
		treeModel6 = glm::scale(treeModel6, glm::vec3(0.005f));

		// Tree6 Texture
		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);
		shaderBlending.SetMat4("model", treeModel6);
		glBindTexture(GL_TEXTURE_2D, treeTexture);

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", treeModel6);
		treeObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// Tree7 position
		glm::mat4 treeModel7 = glm::mat4(1.0f);
		treeModel7 = glm::translate(treeModel7, glm::vec3(-4.2f, -0.46f, -3.0f));
		treeModel7 = glm::scale(treeModel7, glm::vec3(0.005f));

		// Tree7 Texture
		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);
		shaderBlending.SetMat4("model", treeModel7);
		glBindTexture(GL_TEXTURE_2D, treeTexture);

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", treeModel7);
		treeObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// Bush1 position
		glm::mat4 bushModel1 = glm::mat4(1.0f);
		bushModel1 = glm::translate(bushModel1, glm::vec3(-4.5f, -0.50f, -4.4f));
		bushModel1 = glm::scale(bushModel1, glm::vec3(0.009f));
		bushModel1 = glm::rotate(bushModel1, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// Bush1 Texture

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", bushModel1);
		bushObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// Bush2 position
		glm::mat4 bushModel2 = glm::mat4(1.0f);
		bushModel2 = glm::translate(bushModel2, glm::vec3(-1.7f, -0.50f, 3.2f));
		bushModel2 = glm::scale(bushModel2, glm::vec3(0.009f));

		// Bush2 Texture

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", bushModel2);
		bushObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// Bush3 position
		glm::mat4 bushModel3 = glm::mat4(1.0f);
		bushModel3 = glm::translate(bushModel3, glm::vec3(1.0f, -0.50f, -4.0f));
		bushModel3 = glm::scale(bushModel3, glm::vec3(0.009f));
		bushModel3 = glm::rotate(bushModel3, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// Bush3 Texture

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", bushModel3);
		bushObjModel.Draw(lightingShader);

		// Bush4 position
		glm::mat4 bushModel4 = glm::mat4(1.0f);
		bushModel4 = glm::translate(bushModel4, glm::vec3(2.0f, -0.50f, -4.3f));
		bushModel4 = glm::scale(bushModel4, glm::vec3(0.009f));
		bushModel4 = glm::rotate(bushModel4, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// Bush4 Texture

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", bushModel4);
		bushObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// Bush5 position
		glm::mat4 bushModel5 = glm::mat4(1.0f);
		bushModel5 = glm::translate(bushModel5, glm::vec3(-0.4f, -0.50f, 2.8f));
		bushModel5 = glm::scale(bushModel5, glm::vec3(0.009f));
		bushModel5 = glm::rotate(bushModel5, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// Bush5 Texture

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", bushModel5);
		bushObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);


		// Bush6 position
		glm::mat4 bushModel6 = glm::mat4(1.0f);
		bushModel6 = glm::translate(bushModel6, glm::vec3(-4.3f, -0.50f, 4.2f));
		bushModel6 = glm::scale(bushModel6, glm::vec3(0.009f));

		// Bush6 Texture

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", bushModel6);
		bushObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// Bush7 position
		glm::mat4 bushModel7 = glm::mat4(1.0f);
		bushModel7 = glm::translate(bushModel7, glm::vec3(-3.5f, -0.50f, 2.5f));
		bushModel7 = glm::scale(bushModel7, glm::vec3(0.009f));

		// Bush7 Texture

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", bushModel7);
		bushObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// Bush8 position
		glm::mat4 bushModel8 = glm::mat4(1.0f);
		bushModel8 = glm::translate(bushModel8, glm::vec3(-2.5f, -0.50f, 4.4f));
		bushModel8 = glm::scale(bushModel8, glm::vec3(0.009f));

		// Bush8 Texture

		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightingShader.Use();
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.SetMat4("view", pCamera->GetViewMatrix());
		lightingShader.SetMat4("model", bushModel8);
		bushObjModel.Draw(lightingShader);

		shaderBlending.Use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();

	// Clear floor VAO

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		pCamera->ProcessKeyboard(UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		pCamera->ProcessKeyboard(DOWN, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		pCamera->ProcessKeyboard(ROTATELEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		pCamera->ProcessKeyboard(ROTATERIGHT, (float)deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);

	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}

unsigned int CreateTexture(const std::string& strTexturePath)
{
	unsigned int textureId = -1;

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	//stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(strTexturePath.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "Failed to load texture: " << strTexturePath << std::endl;
	}
	stbi_image_free(data);

	return textureId;
}