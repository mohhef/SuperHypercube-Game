#pragma once
class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void bind()const;
	void unbind() const;

private:
	// vertex buffer id
	unsigned int id;
};

