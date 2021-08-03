#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include "Shader.h"
#include "Renderer.h"


// Shader constructor using the filepath
Shader::Shader(const std::string& filePath)
{
	this->filePath = filePath;
	// Creating a program source object to store the shaders at the specified file path
	ShaderProgramSource source = parseShader(filePath);
	id = createShader(source.vertexSource, source.fragmentSource, source.geometrySource);

}

// Destructor
Shader::~Shader()
{
	GLCall(glDeleteProgram(id));
}

// Method for creating the shader and returning its id
unsigned int Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader)
{
	// create shader program
	GLCall(unsigned int program = glCreateProgram());

	// assign object id to each shader
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
	unsigned int gs;
	if (geometryShader != "") {
		gs = compileShader(GL_GEOMETRY_SHADER, geometryShader);
	}

	// attach shaders
	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	if (geometryShader != "") {
		GLCall(glAttachShader(program, gs));
	}

	// link shaders
	GLCall(glLinkProgram(program));
	
	GLint program_linked;

	GLCall(glGetProgramiv(program, GL_LINK_STATUS, &program_linked));
	std::cout << "Program link status: " << program_linked << std::endl;
	if (program_linked != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		GLCall(glGetProgramInfoLog(program, 1024, &log_length, message));
		std::cout << "Failed to link program" << std::endl;
		std::cout << message << std::endl;
	}
	
	GLCall(glValidateProgram(program));

	// since they have been linked already we can delete
	glDeleteShader(vs);
	glDeleteShader(fs);
	if (geometryShader != "") {
		glDeleteShader(gs);
	}

	return program;
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source) {

	// assign object id
	unsigned int id = glCreateShader(type);
	
	// returns a pointer to the data
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	// verify for errors
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		
		// lets you allocate on the stack dynamically
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Error: Unable to compile" << (type == GL_VERTEX_SHADER ? "VERTEX" : "Fragment") << std::endl;
	}

	return id;
}

// Method for parsing the shader at the specified file path
ShaderProgramSource Shader::parseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	std::string line;
	std::stringstream ss[3];
	ShaderType type = ShaderType::NONE;
	bool hasGeometry = false;

	// Read through the program line by line
	while (getline(stream, line)) {

		// If the line contains #shader vertex or #shader fragment, add the following lines to the appropriate array for storage
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
			else if (line.find("geometry")) {
				type = ShaderType::GEOMETRY;
				hasGeometry = true;
			}
		}
		else
		{
			ss[(int)type] << line << "\n";
		}
	}
	if (hasGeometry) return { ss[0].str(), ss[1].str(), ss[2].str() };
	return { ss[0].str(), ss[1].str(), "" };
}

// Bind the shader
void Shader::bind() const
{
	GLCall(glUseProgram(id));
}

// unbind the shader
void Shader::unbind() const
{
	GLCall(glUseProgram(0));
}

void Shader::setUniform1i(const std::string& name, int i) {
	glUniform1i(getUniformLocation(name), i);
}

void Shader::setUniform1f(const std::string& name, float i) {
	glUniform1f(getUniformLocation(name), i);
}

void Shader::setUniform4f(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(getUniformLocation(name), x, y, z, w);
}

void Shader::setUniform3Vec(const std::string& name, glm::vec3 vec)
{
	glUniform3fv(getUniformLocation(name),1,&vec[0]);
}

void Shader::setUniform4Vec(const std::string& name, glm::vec4 vec) 
{
	glUniform4fv(getUniformLocation(name), 1, &vec[0]);
}

void Shader::setUniform4Mat(const std::string& name, glm::mat4 mat) 
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

unsigned int Shader::getUniformLocation(const std::string& name)
{
	if (uniformLocationCache.find(name) != uniformLocationCache.end())
		return uniformLocationCache[name];
	GLCall(int location = glGetUniformLocation(id, name.c_str()));
	if (location == -1)
		std::cout << "Error: unable to locate uniform " << name << std::endl;
	uniformLocationCache[name] = location;
	return location;
}
