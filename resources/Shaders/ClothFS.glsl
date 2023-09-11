#version 460 core

out vec4 color;

in vec3 position;
in vec2 texCoord;
in vec3 normal;

// Texture Sampler
uniform sampler2D uniTex;

uniform vec3 uniLightPos;
uniform vec3 uniLightColor;

void main()
{
    // Ambient
    float ambientStrength = 0.4f;
    vec3 ambient = ambientStrength * uniLightColor;

    // ambient points
    ambientStrength = 0.7f;
    vec3 light_col_intensity = uniLightColor * (1.0f/(pow(distance(position, uniLightPos), 2.0f)));
    ambient += ambientStrength * light_col_intensity;
    

    // diffuse points
    vec3 norm = normalize(normal);
    
    light_col_intensity = uniLightColor * (1.0f/(pow(distance(position, uniLightPos), 2.0f)));
    vec3 lightDir = normalize(uniLightPos - position);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * light_col_intensity;

    vec3 specular = vec3(0.0);
   

    // final color
    vec4 t_col4 = texture(uniTex, texCoord);
//    vec4 t_col4 = vec4(1.0, 1.0, 1.0, 1.0);
    vec3 t_col = vec3(t_col4.x, t_col4.y, t_col4.z);
    vec3 result = (ambient + diffuse + specular) * t_col;
    color = vec4(result, 1.0f);
    
//    color = vec4(1.0, 1.0, 1.0, 1.0);
}
