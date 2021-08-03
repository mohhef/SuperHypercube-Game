#include "Texture.h"
#include "stb_image.h"
#include "Renderer.h"

Texture::Texture(const string& path)
	:filePath(path), data(nullptr), width(0), height(0), nrChannels(0)
{
	// step 1: create and bind textures
	GLCall(glGenTextures(1, &id));
	GLCall(glBindTexture(GL_TEXTURE_2D, id));

	// step 2: filter paramaters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// step 3: load textures
	stbi_set_flip_vertically_on_load(1);
	data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (!data)
	{
		cout << "Texture failed to load at path: " << path << endl;
		stbi_image_free(data);
	}

	// step 4: upload texture to GPU
	GLenum format;
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if (nrChannels == 4)
		format = GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// step 5: free all resources
	stbi_image_free(data);
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &id));
}
void Texture::bind(unsigned int slot) const {
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, id));
}

void Texture::unbind() const {
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}