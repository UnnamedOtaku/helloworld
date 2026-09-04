#version 330 core

out vec4 FragColor;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec3 vertexColor;
in vec2 TexCoord;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewPos;
  
uniform Material material;
uniform Light light;

void main()
{
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
  	
    // diffuse 
    vec3 norm = normalize(vertexNormal);
    vec3 lightDir = normalize(light.position - vertexPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    
    // specular
    vec3 viewDir = normalize(viewPos - vertexPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord)); 

    // emission
    vec3 emission = vec3(texture(material.specular, TexCoord)) == vec3(0.0) ? vec3(texture(material.emission, TexCoord)) : vec3(0.0);
        
    vec3 result = ambient + diffuse + specular + emission;
    
    FragColor = vec4(result, 1.0);
}