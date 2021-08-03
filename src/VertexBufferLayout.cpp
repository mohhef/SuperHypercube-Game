#include "VertexBufferLayout.h"
#include "Renderer.h"


// Method to get the size of the associated type
unsigned int VertexBufferElement::getSizeOfType(unsigned int type) 
{
	switch (type)
	{
	case GL_FLOAT: return 4;
	case GL_UNSIGNED_INT: return 4;
	case GL_UNSIGNED_BYTE: return 1;
	}
	ASSERT(false);
	return 0;
}

// Constructor for vertex buffer layout
VertexBufferLayout::VertexBufferLayout()
{
	stride = 0;
}

// Get elements
const std::vector<VertexBufferElement> VertexBufferLayout::getElements() const
{
	return elements;
}

// Get the stride of the layout
unsigned int VertexBufferLayout::getStride() const
{
	return stride;
}

// Template for push method
template<typename T>
void VertexBufferLayout::push(unsigned int count) 
{
	static_assert(false);
}

// Push a float with a certain count to buffer layout
template<>
void VertexBufferLayout::push<float>(unsigned count) 
{
	elements.push_back({ GL_FLOAT,count, GL_FALSE });
	stride += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
}

// Push an unsigned int with a certain count to buffer layout
template<>
void VertexBufferLayout::push<unsigned int>(unsigned int count) 
{
	elements.push_back({ GL_UNSIGNED_INT,count, GL_FALSE });
	stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
}

// Push an unsigned char with a certain count to buffer layout
template<>
void VertexBufferLayout::push<unsigned char>(unsigned int count) 
{
	elements.push_back({ GL_UNSIGNED_BYTE,count, GL_TRUE });
	stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
}