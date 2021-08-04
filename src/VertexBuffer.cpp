#include "VertexBuffer.h"
#include "Renderer.h"

// Vertex buffer constructor
VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

// Vertex buffer constructor (dynamic)
VertexBuffer::VertexBuffer(unsigned int size)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
}

// Vertex buffer destructor
VertexBuffer::~VertexBuffer()
{
	GLCall(glDeleteBuffers(1, &id));
}

// Vertex buffer bind
void VertexBuffer::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, id);
}

// Vertex buffer unbind
void VertexBuffer::unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
