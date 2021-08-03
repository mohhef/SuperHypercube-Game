	#pragma once
#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void bind() const;
	void unbind() const;
	void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

private:
	// vertex array id
	unsigned int id;
};

