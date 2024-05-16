#define GLM_FORCE_CTOR_INIT 
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

// for easier usage
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;



#include <stdlib.h> 
#include <stdio.h>
#include <math.h> 
#include <vector>
#include <thread>
#include <chrono>

#include <Windows.h>
#include <mmsystem.h>
#include <locale>
#include <codecvt>

#include <ctime>
#include <cstdlib>

#include <GL/glew.h>

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
#include "KartModel.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "winmm.lib")

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static bool isNight = false;

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
	-100.0f, -100.0f,  100.0f, // Vertex 0: Bottom-left-front corner
	 100.0f, -100.0f,  100.0f, // Vertex 1: Bottom-right-front corner
	 100.0f, -100.0f, -100.0f, // Vertex 2: Bottom-right-back corner
	-100.0f, -100.0f, -100.0f, // Vertex 3: Bottom-left-back corner
	-100.0f,  100.0f,  100.0f, // Vertex 4: Top-left-front corner
	 100.0f,  100.0f,  100.0f, // Vertex 5: Top-right-front corner
	 100.0f,  100.0f, -100.0f, // Vertex 6: Top-right-back corner
	-100.0f,  100.0f, -100.0f  // Vertex 7: Top-left-back corner
};
unsigned int skyboxIndices[] =
{
	// Right
	6, 5, 1,
	1, 2, 6,
	// Left
	7, 3, 0,
	0, 4, 7,
	// Top
	6, 7, 4,
	4, 5, 6,
	// Bottom
	2, 1, 0,
	0, 3, 2,
	// Back
	5, 4, 0,
	0, 1, 5,
	// Front
	6, 2, 3,
	3, 7, 6
};


void PlayMusic(std::string path)
{

	std::string soundFile = path;
	std::wstring stemp = std::wstring(soundFile.begin(), soundFile.end());
	LPCWSTR sw = stemp.c_str();

	PlaySound(sw, NULL, SND_ASYNC | SND_LOOP);
	std::cout << "Music is playing" << std::endl;
}

void drawObject(glm::vec3 position, glm::vec3 scale, float rotation, Shader& shaderBlending, Shader& lightingShader, Model& objModel, glm::mat4 projection, glm::mat4 view, glm::vec3 lightPos, Camera* pCamera) {
	// Object position
	glm::mat4 objModelMat = glm::mat4(1.0f);
	objModelMat = glm::translate(objModelMat, position);
	objModelMat = glm::scale(objModelMat, scale);
	objModelMat = glm::rotate(objModelMat, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));

	// Object Texture
	lightingShader.Use();
	lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
	lightingShader.SetVec3("lightPos", lightPos);
	lightingShader.SetVec3("viewPos", pCamera->GetPosition());
	lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
	lightingShader.SetMat4("view", pCamera->GetViewMatrix());
	lightingShader.SetMat4("model", objModelMat);

	objModel.Draw(lightingShader);

	shaderBlending.Use();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	shaderBlending.SetMat4("projection", projection);
	shaderBlending.SetMat4("view", view);
}

void drawObjectWithTexture(glm::vec3 position, glm::vec3 scale, float rotation, GLuint texture, Shader& shaderBlending, Shader& lightingShader, Model& objModel, glm::mat4 projection, glm::mat4 view, glm::vec3 lightPos, Camera* pCamera) {
	// Object position
	glm::mat4 objModelMat = glm::mat4(1.0f);
	objModelMat = glm::translate(objModelMat, position);
	objModelMat = glm::scale(objModelMat, scale);
	objModelMat = glm::rotate(objModelMat, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));

	// Object Texture
	lightingShader.Use();
	lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
	lightingShader.SetVec3("lightPos", lightPos);
	lightingShader.SetVec3("viewPos", pCamera->GetPosition());
	lightingShader.SetMat4("projection", pCamera->GetProjectionMatrix());
	lightingShader.SetMat4("view", pCamera->GetViewMatrix());
	lightingShader.SetMat4("model", objModelMat);

	// Bind the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	objModel.Draw(lightingShader);

	shaderBlending.Use();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	shaderBlending.SetMat4("projection", projection);
	shaderBlending.SetMat4("view", view);
}

void drawFloor(const glm::mat4& projection, const glm::mat4& view, GLuint floorVAO, GLuint floorTexture, Shader& shaderFloor) {
	glm::mat4 model = glm::mat4(1.0f);
	shaderFloor.Use();
	shaderFloor.SetMat4("projection", projection);
	shaderFloor.SetMat4("view", view);
	glBindVertexArray(floorVAO);
	glBindTexture(GL_TEXTURE_2D, floorTexture);
	shaderFloor.SetMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Day()
{
	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();

	std::string facesCubemap[6] =
	{
		parentDir + "/resources/skybox/right (1).jpg",
		parentDir + "/resources/skybox/left (1).jpg",
		parentDir + "/resources/skybox/top (1).jpg",
		parentDir + "/resources/skybox/bottom (1).jpg",
		parentDir + "/resources/skybox/front (1).jpg",
		parentDir + "/resources/skybox/back (1).jpg"
	};

	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}
}

void Night()
{
	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();

	std::string facesCubemapNight[6] =
	{
		parentDir + "/resources/skybox/right1.jpg",
		parentDir + "/resources/skybox/left1.jpg",
		parentDir + "/resources/skybox/top1.jpg",
		parentDir + "/resources/skybox/bottom1.jpg",
		parentDir + "/resources/skybox/front1.jpg",
		parentDir + "/resources/skybox/back1.jpg"

	};

	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemapNight[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemapNight[i] << std::endl;
			stbi_image_free(data);
		}
	}
}






int main(int argc, char** argv)
{


	std::string strFullExeFileName = argv[0];
	std::string strExePath;
	KartModel kart;
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glewInit();


	// Generates Shader objects
	Shader shaderProgram("Blending.vs", "Blending.fs");
	Shader skyboxShader("Skybox.vs", "Skybox.fs");

	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos(0.0f, 3.0f, 1.0f);
	shaderProgram.Use();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	skyboxShader.Use();
	glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);


	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
	std::string backgroundMusic = parentDir + "/resources/sounds/background.wav";
	PlayMusic(backgroundMusic);






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

	std::string GoombaObjFileName = (currentPath + "\\resources\\models\\Goomba\\goomba.obj");
	Model goombaObjModel(GoombaObjFileName, false);

	std::string MarioObjFileName = (currentPath + "\\resources\\models\\Mario\\mario.obj");
	Model marioObjModel(MarioObjFileName, false);







	// Create VAO, VBO, and EBO for the skybox
	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// All the faces of the cubemap (make sure they are in this exact order)





	// Creates the cubemap texture object
	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// This might help with seams on some systems
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Cycles through all the textures and attaches them to the cubemap object

	Day();





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

		// Handle kart input and update kart state
		kart.handleInput(window, static_cast<float>(deltaTime));
		kart.update(static_cast<float>(deltaTime));

		// Get the new kart position and direction
		glm::vec3 kartPosition = kart.getPosition();
		float kartDirection = kart.getDirection();

		// Calculate the camera's position just behind the kart
		float distanceBehind = 0.6f; // Adjust this distance as needed
		glm::vec3 cameraOffset = glm::vec3(-distanceBehind * sin(glm::radians(kartDirection)), 0.5f, -distanceBehind * cos(glm::radians(kartDirection)));
		glm::vec3 cameraPosition = kartPosition + cameraOffset;

		// Calculate the camera's target position, slightly in front of the kart
		float distanceAhead = 0.5f; // Adjust this distance as needed
		glm::vec3 targetOffset = glm::vec3(distanceAhead * sin(glm::radians(kartDirection)), 0.0f, distanceAhead * cos(glm::radians(kartDirection)));
		glm::vec3 targetPosition = kartPosition + targetOffset;

		// Calculate the direction from the camera position to the target position
		glm::vec3 cameraDirection = glm::normalize(targetPosition - cameraPosition);

		// Set the camera's position and direction
		pCamera->SetPosition(cameraPosition);
		pCamera->SetDirection(cameraDirection);

		// Render the scene with the updated camera position and direction
		glm::mat4 projection = pCamera->GetProjectionMatrix();
		glm::mat4 view = pCamera->GetViewMatrix();



		//Floor
		drawFloor(projection, view, floorVAO, floorTexture, shaderFloor);


		glm::mat4 kartModel = kart.getModelMatrix();

		glm::vec3 kartScale = glm::vec3(0.001f);


		// Models


		// Kart
		drawObjectWithTexture(kartPosition, kartScale, kartDirection, kartTexture, shaderBlending, lightingShader, kartObjModel, projection, view, lightPos, pCamera);

		// Mario
		glm::vec3 marioPosition = glm::vec3(4.6f, -0.4f, -1.0f);
		glm::vec3 marioScale = glm::vec3(0.005f);
		float marioRotation = 0.0f;

		drawObject(marioPosition, marioScale, marioRotation, shaderBlending, lightingShader, marioObjModel, projection, view, lightPos, pCamera);


		// Trees

		// Universal tree related
		glm::vec3 treeScale = glm::vec3(0.005f);
		float treeRotation = 0.0f;


		// Tree1 	
		glm::vec3 treePosition = glm::vec3(3.0f, -0.46f, -1.0f);
		drawObjectWithTexture(treePosition, treeScale, treeRotation, treeTexture, shaderBlending, lightingShader, treeObjModel, projection, view, lightPos, pCamera);

		// Tree2
		glm::vec3 tree2Position = glm::vec3(-0.5f, -0.46f, -2.0f);
		drawObjectWithTexture(tree2Position, treeScale, treeRotation, treeTexture, shaderBlending, lightingShader, treeObjModel, projection, view, lightPos, pCamera);

		// Tree3
		glm::vec3 treePosition3 = glm::vec3(4.5f, -0.46f, 4.5f);
		drawObjectWithTexture(treePosition3, treeScale, treeRotation, treeTexture, shaderBlending, lightingShader, treeObjModel, projection, view, lightPos, pCamera);

		// Tree4
		glm::vec3 treePosition4 = glm::vec3(4.5f, -0.46f, -4.5f);
		drawObjectWithTexture(treePosition4, treeScale, treeRotation, treeTexture, shaderBlending, lightingShader, treeObjModel, projection, view, lightPos, pCamera);

		// Tree5
		glm::vec3 treePosition5 = glm::vec3(-3.5f, -0.46f, 4.2f);
		drawObjectWithTexture(treePosition5, treeScale, treeRotation, treeTexture, shaderBlending, lightingShader, treeObjModel, projection, view, lightPos, pCamera);

		// Tree6
		glm::vec3 treePosition6 = glm::vec3(-4.2f, -0.46f, 3.0f);
		drawObjectWithTexture(treePosition6, treeScale, treeRotation, treeTexture, shaderBlending, lightingShader, treeObjModel, projection, view, lightPos, pCamera);

		// Tree7
		glm::vec3 treePosition7 = glm::vec3(-4.2f, -0.46f, -3.0f);
		drawObjectWithTexture(treePosition7, treeScale, treeRotation, treeTexture, shaderBlending, lightingShader, treeObjModel, projection, view, lightPos, pCamera);



		// Bushes

		// Universal bush related	
		glm::vec3 bushScale = glm::vec3(0.009f);
		float bushRotation = 0.0f;


		// Bush1
		glm::vec3 bushPosition1 = glm::vec3(-4.5f, -0.50f, -4.4f);
		drawObject(bushPosition1, bushScale, bushRotation, shaderBlending, lightingShader, bushObjModel, projection, view, lightPos, pCamera);


		// Bush2
		glm::vec3 bushPosition2 = glm::vec3(-1.7f, -0.50f, 3.2f);
		drawObject(bushPosition2, bushScale, bushRotation, shaderBlending, lightingShader, bushObjModel, projection, view, lightPos, pCamera);

		// Bush3
		glm::vec3 bushPosition3 = glm::vec3(1.0f, -0.50f, -4.0f);
		drawObject(bushPosition3, bushScale, bushRotation, shaderBlending, lightingShader, bushObjModel, projection, view, lightPos, pCamera);



		// Bush4
		glm::vec3 bushPosition4 = glm::vec3(2.0f, -0.50f, -4.3f);
		drawObject(bushPosition4, bushScale, bushRotation, shaderBlending, lightingShader, bushObjModel, projection, view, lightPos, pCamera);


		// Bush5
		glm::vec3 bushPosition5 = glm::vec3(-0.4f, -0.50f, 2.8f);
		drawObject(bushPosition5, bushScale, bushRotation, shaderBlending, lightingShader, bushObjModel, projection, view, lightPos, pCamera);


		// Bush 6
		glm::vec3 bushPosition6 = glm::vec3(-4.3f, -0.50f, 4.2f);
		drawObject(bushPosition6, bushScale, bushRotation, shaderBlending, lightingShader, bushObjModel, projection, view, lightPos, pCamera);


		// Bush 7
		glm::vec3 bushPosition7 = glm::vec3(-3.5f, -0.50f, 2.5f);
		drawObject(bushPosition7, bushScale, bushRotation, shaderBlending, lightingShader, bushObjModel, projection, view, lightPos, pCamera);

		// Bush 8
		glm::vec3 bushPosition8 = glm::vec3(-2.5f, -0.50f, 4.4f);
		drawObject(bushPosition8, bushScale, bushRotation, shaderBlending, lightingShader, bushObjModel, projection, view, lightPos, pCamera);





		// Universal goomba related
		glm::vec3 goombaScale = glm::vec3(0.002f);
		float goombaRotation = 90.0f;



		// Goomba1
		glm::vec3 goombaPosition1 = glm::vec3(-2.0f, -0.50f, 4.6f);
		drawObject(goombaPosition1, goombaScale, goombaRotation, shaderBlending, lightingShader, goombaObjModel, projection, view, lightPos, pCamera);

		// Goomba2
		glm::vec3 goombaPosition2 = glm::vec3(-3.7f, -0.50f, 3.6f);
		drawObject(goombaPosition2, goombaScale, goombaRotation, shaderBlending, lightingShader, goombaObjModel, projection, view, lightPos, pCamera);

		// Goomba3
		glm::vec3 goombaPosition3 = glm::vec3(-4.0f, -0.50f, 2.0f);
		drawObject(goombaPosition3, goombaScale, goombaRotation, shaderBlending, lightingShader, goombaObjModel, projection, view, lightPos, pCamera);

		// Since the cubemap will always have a depth of 1.0, we need that equal sign so it doesn't get discarded
		glDepthFunc(GL_LEQUAL);

		skyboxShader.Use();

		// We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
		// The last row and column affect the translation of the skybox (which we don't want to affect)
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
		// where an object is present (a depth of 1.0f will always fail against any object's depth value)
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Switch back to the normal depth function
		glDepthFunc(GL_LESS);

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
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		pCamera->ProcessKeyboard(UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
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


	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		isNight = !isNight; // Toggle state

		if (isNight)
		{
			Night();
		}
		else
		{
			Day();
		}
	}


	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
	{
		static int savedWidth = 0, savedHeight = 0, savedPosX = 0, savedPosY = 0;
		static bool isFullscreen = false;

		if (!isFullscreen)
		{
			// Save the current window size and position
			glfwGetWindowSize(window, &savedWidth, &savedHeight);
			glfwGetWindowPos(window, &savedPosX, &savedPosY);

			// Get the monitor and its video mode
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			// Set the window to fullscreen
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

			isFullscreen = true;
		}
		else
		{
			// Restore the window's original size and position
			glfwSetWindowMonitor(window, NULL, savedPosX, savedPosY, savedWidth, savedHeight, 0);

			isFullscreen = false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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