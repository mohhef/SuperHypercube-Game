#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>

struct ShaderProgramSource {
	std::string vertexSource;
	std::string fragmentSource;
	std::string geometrySource;
};

enum class ShaderType
{
	NONE = -1, 
	VERTEX = 0, 
	FRAGMENT = 1,
	GEOMETRY = 2
};

class Shader 
{
public:
	Shader(const std::string& v);
	~Shader();

	void bind() const;
	void unbind() const;

	unsigned int getUniformLocation(const std::string& name);
	void setUniform1i(const std::string& name, int i);
	void setUniform1f(const std::string& name, float i);
	void setUniform4f(const std::string& name,  float x, float y, float z, float w) ;
	void setUniform3Vec(const std::string& name, glm::vec3 vec);
	void setUniform4Vec(const std::string& name, glm::vec4 vec) ;
	void setUniform4Mat(const std::string& name, glm::mat4 mat) ;

	unsigned int  compileShader(unsigned int type, const std::string& source);
	ShaderProgramSource parseShader(const std::string& filepath);
	unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader);

private:
	std::string filePath;
	unsigned int id;
	std::unordered_map<std::string, int> uniformLocationCache;
};