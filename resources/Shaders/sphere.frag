#version 460 core

in vec3 position;
in vec3 normal;

out vec4 color;


uniform vec3 uniLightPos;
uniform vec3 uniLightColor;

void main()
{
    // Ambient
    float ambientStrength = 0.5f;
    vec3 ambient = ambientStrength * uniLightColor;

    // Diffuse
    vec3 lightDir = normalize(uniLightPos - position);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uniLightColor;

    color = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    vec3 objectColor = vec3(color.x, color.y, color.z);
    vec3 result = (ambient + diffuse) * objectColor;
    color = vec4(result, 1.0f);
}