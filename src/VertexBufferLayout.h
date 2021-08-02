#pragma once
#include <vector>
#include <GL/glew.h>

struct VertexBufferElement {
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static unsigned int getSizeOfType(unsigned int type);
};

class VertexBufferLayout
{
public:
	VertexBufferLayout();

	template<typename T>
	void push(unsigned int count);

	template<>
	void push<float>(unsigned count);

	template<>
	void push<unsigned int>(unsigned int count);

	template<>
	void push<unsigned char>(unsigned int count);

	const std::vector<VertexBufferElement> getElements() const;
	
	unsigned int getStride() const;

private:
	std::vector<VertexBufferElement> elements;
	unsigned int stride;
};