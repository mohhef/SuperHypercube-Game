#shader vertex
#version 330 core

layout(location = 0) in vec4 vertex; 

out vec2 textPos;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    textPos = vertex.zw;
}

#shader fragment
#version 330 core

in vec2 textPos;
out vec4 textColor;

uniform sampler2D text;
uniform vec3 ourColor;

void main()
{
    vec4 result = vec4(1.0, 1.0, 1.0, texture(text, textPos).r);
    textColor = vec4(ourColor, 1.0) * result;
}