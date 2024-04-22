#pragma once
#include<GL/glew.h>
#include <glm.hpp>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

std::string get_file_contents(const char* filename);

class Shader
{
public:
	// Reference ID of the Shader Program
	GLuint ID;
	// Constructor that build the Shader Program from 2 different shaders
	Shader(const char* vertexFile, const char* fragmentFile);

	// Activates the Shader Program
	void Activate();
	// Deletes the Shader Program
	void Delete();

	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetVec2(const std::string& name, const glm::vec2& value) const;
	void SetVec3(const std::string& name, const glm::vec3& value) const;
	void SetVec4(const std::string& name, const glm::vec4& value) const;
	void SetMat4(const std::string& name, const glm::mat4& value) const;

private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);
};

