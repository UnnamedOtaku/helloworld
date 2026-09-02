#version 330 core

out vec4 FragColor;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec3 vertexColor;
in vec2 TexCoord;

uniform sampler2D Texture;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    // ----- CONFIGURACIÓN DE LA LUZ -----
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    // ----- NORMALIZAR VECTORES -----
    vec3 normal = normalize(vertexNormal);
    vec3 lightDir = normalize(lightPos - vertexPos);
    
    // ----- ILUMINACIÓN DIFUSA (Lambert) -----
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // ----- LUZ AMBIENTE -----
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
    
    // ----- (OPCIONAL) ESPECULAR (Blinn-Phong) -----
    // Si quieres brillos, descomenta esto:
    /*
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - vertexPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    */
    
    // ----- COMBINAR ILUMINACIÓN -----
    vec3 lighting = ambient + diffuse; // + specular (si lo descomentas)
    
    // ----- OBTENER COLOR DE TEXTURA -----
    vec4 texColor = texture(Texture, TexCoord);
    
    // ----- COLOR FINAL -----
    FragColor = texColor * vec4(vertexColor * lighting, 1.0);
}