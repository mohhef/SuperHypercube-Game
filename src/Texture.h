#pragma once
#include <iostream>
using namespace std;

class Texture 
{
public:
	unsigned int id; // texture id
	Texture(const std::string& path);
	~Texture();
	
	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int getWidth() const { return width; };
	inline int getHeight() const { return height; };

private:
	string filePath; // file path to texture
	unsigned char* data; // holds all data
	int width, height, nrChannels; 
};