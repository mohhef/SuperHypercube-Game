#shader vertex
// These shaders were heavily influenced by learnOpengl.com: https://learnopengl.com/
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int invertStatus;

out vec3 FragPos;
out vec3 Normal;
out vec2 Texture;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0); // model's position with respect to perspective and view

    FragPos = vec3(model * vec4(aPos, 1.0)); // model's position with respect to global axis
    
    vec3 nor;

    if (invertStatus == 1) {
        nor = aNormal * vec3(-1.0f, -1.0f, -1.0f);
    }
    else {
        nor = aNormal;
    }

    Normal = mat3(transpose(inverse(model))) * nor; // model's normal vector
    Texture = aTexture;
}

#shader fragment
#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 Texture;

uniform vec3 ourColor;

uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPos;

uniform sampler2D textureObject;
uniform sampler2D texture_diffuse1;
uniform samplerCube depthMap;

uniform float map_range;
uniform bool drawShadows;

uniform int textureStatus; // 1 indicates texture is being applied
uniform int textureDiffStatus; // 1 indicates 3D model texture is being applied
uniform int shininess;


// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
    );

float ShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPosition;
    // use the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // currently in linear range between [0,1], so re-transform it back to original depth value
    closestDepth *= map_range;
    // now get length between the fragment and light position
    float currentDepth = length(fragToLight);
    // PCF
     float shadow = 0.0;
     float bias = 1.0; 
     float samples = 4.0;
     float offset = 0.1;
     for(float x = -offset; x < offset; x += offset / (samples * 0.5))
     {
         for(float y = -offset; y < offset; y += offset / (samples * 0.5))
         {
             for(float z = -offset; z < offset; z += offset / (samples * 0.5))
             {
                 float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
                 closestDepth *= map_range;   // Undo mapping [0;1]
                 if(currentDepth - bias > closestDepth)
                     shadow += 1.0;
             }
         }
     }
     shadow /= (samples * samples * samples);

    return shadow;
}

void main()
{
    vec3 color;



    if (textureStatus == 1)
    {
        // determine result with texture applied
        color = ourColor * texture(textureObject, Texture).rgb;
    }
    else if (textureDiffStatus == 1) {
        color = ourColor * texture(texture_diffuse1, Texture).rgb;
    }
    else
    {
        // determine result without texture applied
        color = ourColor;
    }
    vec3 ambientVal = 0.15 * color;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(lightPosition - FragPos);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuseVal = diff * lightColor;

    // specular
    float specularFactor = 0.5;
    vec3 viewDirection = normalize(viewPos - FragPos);
    vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    float spec = pow(max(dot(norm, halfwayDirection), 0.0), shininess);
    vec3 specularVal = specularFactor * spec * lightColor;


    float shadow = drawShadows ? ShadowCalculation(FragPos) : 0.0;
    vec3 lighting = (ambientVal + (1.0 - shadow) * (diffuseVal + specularVal)) * color;

    FragColor = vec4(lighting, 1.0f);
}
