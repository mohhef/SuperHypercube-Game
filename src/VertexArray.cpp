#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Renderer.h"

//Vertex Array constructor
VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &id));
}

// Vertex array destructor
VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &id));
}

// Define layout of the Vertex Buffer for the vertex array
void VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
	// bind the vertex buffer and array
	bind();
	vb.bind();
	const auto& elements = layout.getElements();
	unsigned int offset = 0;

	// for each element in the layout, define the vertex array pointers
	for (unsigned int i = 0; i < elements.size(); i++) {
		const auto& element = elements[i];
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset));
		offset += element.count * VertexBufferElement::getSizeOfType(element.type);
	}

}

// bind the vertex array
void VertexArray::bind() const
{
	GLCall(glBindVertexArray(id));
}

// unbind the vertex array
void VertexArray::unbind() const
{
	GLCall(glBindVertexArray(0));
}
