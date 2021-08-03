#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 Texture;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0); // model's position with respect to perspective and view

    FragPos = vec3(model * vec4(aPos, 1.0)); // model's position with respect to global axis
    Normal = mat3(transpose(inverse(model))) * aNormal; // model's normal vector
    Texture = aTexture;
}

#shader fragment
#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 Texture;

struct Material 
{
    float shiny;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Light 
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform Material material;
uniform Light light;

void main()
{
    //ambient
    vec3 ambientVal = light.ambient * material.ambient;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuseVal = diff * material.diffuse * light.diffuse;

    // specular
    vec3 viewDirection = normalize(viewPos - FragPos);
    vec3 reflectDirection = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shiny);
    vec3 specularVal = spec * material.specular * light.specular;

    vec3 result = (ambientVal + diffuseVal + specularVal);
    FragColor = vec4(result, 1.0f);
}
