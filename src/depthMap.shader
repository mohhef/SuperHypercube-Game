#shader vertex
// These shaders were heavily influenced by learnOpengl.com: https://learnopengl.com/
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
}

#shader geometry
#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

// Cube faces for which we will use to draw the depth map in all 6 directions from the point light.
uniform mat4 shadowCubeFaces[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    // for each face emit a depth map
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // a built-in variable that specifies to which face we render.
        for (int i = 0; i < 3; ++i) // for each of the triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowCubeFaces[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}

#shader fragment
#version 330 core

in vec4 FragPos;

// Position of the light
uniform vec3 lightPosition;
// The distance for which we will render the light
uniform float map_range;

void main()
{
    // Distance from the light
    float lightDistance = length(FragPos.xyz - lightPosition);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / map_range;

    // write the distance from the light as the measured depth
    gl_FragDepth = lightDistance;
}