#shader vertex
#version 330 core

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = vec4(position,1.0f);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(0.2, 0.3, 0.8, 1.0);
}