#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <iostream>

#include "Shader.h"

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 lightModel = glm::mat4(1.0f);
glm::mat4 groundModel = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 proj;

glm::vec3 lightPos(1.2f, 5.0f,  2.0f);
glm::vec3 groundPos(0.0f, -1.0f, 0.0f);

float deltaTime = 0.0f;
double lastTime = 0.0;

bool firstMouse = true;
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX = 400, lastY = 300;
float fov   =  45.0f;

bool showNormals = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
        showNormals = !showNormals;
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime; // adjust accordingly

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraSpeed *= 5.0f; // Increase speed when left control is pressed
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void generateSphereSmooth(
    float radius, int sectors, int stacks,
    std::vector<GLfloat>& vertices,
    std::vector<GLuint>& indices
)
{
    // Limpiar vectores
    vertices.clear();
    indices.clear();
    
    // Constantes
    const float PI = glm::pi<float>();
    const float TWO_PI = glm::two_pi<float>();
    
    // Reservar espacio para mejorar rendimiento
    int numVertices = (stacks + 1) * (sectors + 1);
    int numIndices = stacks * sectors * 6;
    vertices.reserve(numVertices * 11);  // 11 floats por vértice (x,y,z, nx,ny,nz, r,g,b, u,v)
    indices.reserve(numIndices);
    
    // Generar vértices (todos únicos, pero compartidos entre triángulos)
    for (int i = 0; i <= stacks; ++i) {
        float phi = PI / 2.0f - (i * PI / stacks);  // Ángulo vertical: desde PI/2 hasta -PI/2
        float y = radius * glm::sin(phi);
        float radius_xy = radius * glm::cos(phi);
        
        for (int j = 0; j <= sectors; ++j) {
            float theta = j * TWO_PI / sectors;  // Ángulo horizontal: 0 a 2*PI
            
            // Posición
            glm::vec3 position(
                radius_xy * glm::cos(theta),
                y,
                radius_xy * glm::sin(theta)
            );
            
            // Normal = vector posición normalizado (para esfera perfecta)
            glm::vec3 normal = glm::normalize(position);
            
            // Coordenadas de textura
            float u = (float)j / sectors;
            float v = (float)i / stacks;
            
            // Añadir al array plano: [x, y, z, nx, ny, nz, r, g, b, u, v]
            vertices.push_back(position.x);   // posición x
            vertices.push_back(position.y);   // posición y
            vertices.push_back(position.z);   // posición z
            vertices.push_back(normal.x);     // normal x
            vertices.push_back(normal.y);     // normal y
            vertices.push_back(normal.z);     // normal z
            vertices.push_back(1.0f);         // color r (blanco)
            vertices.push_back(1.0f);         // color g (blanco)
            vertices.push_back(1.0f);         // color b (blanco)
            vertices.push_back(u);            // texcoord u
            vertices.push_back(v);            // texcoord v
        }
    }
    
    // Generar índices para triángulos (counter-clockwise)
    // Todos los vértices son compartidos, usamos índices para referenciarlos
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            int current = i * (sectors + 1) + j;
            int next = current + sectors + 1;
            
            // Vértices del cuadrilátero:
            // 0 = current (abajo-izquierda)
            // 1 = current + 1 (abajo-derecha)
            // 2 = next + 1 (arriba-derecha)
            // 3 = next (arriba-izquierda)
            
            // Triángulo 1: (0, 1, 2)
            indices.push_back(current);      // 0
            indices.push_back(current + 1);  // 1
            indices.push_back(next + 1);     // 2
            
            // Triángulo 2: (0, 2, 3)
            indices.push_back(current);      // 0
            indices.push_back(next + 1);     // 2
            indices.push_back(next);         // 3
        }
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    } 

    glViewport(0, 0, 800, 600);

    std::vector<GLfloat> sphereVert;
    std::vector<GLuint> sphereIdx;

    std::vector<GLfloat> groundVert = {
        -0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
         0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  10.0f, 0.0f,
         0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  10.0f, 10.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f,  10.0f
    };

    std::vector<GLuint> groundIdx = {
        0, 1, 2,
        0, 2, 3
    };

    generateSphereSmooth(1.0f, 20, 15, sphereVert, sphereIdx);

    Shader shaderProgram;
    shaderProgram.add("assets/shaders/default.vert", GL_VERTEX_SHADER);
    shaderProgram.add("assets/shaders/default.frag", GL_FRAGMENT_SHADER);
    shaderProgram.add("assets/shaders/default.geom", GL_GEOMETRY_SHADER);
    shaderProgram.link();

    Shader lightShaderProgram;
    lightShaderProgram.add("assets/shaders/default.vert", GL_VERTEX_SHADER);
    lightShaderProgram.add("assets/shaders/light.frag", GL_FRAGMENT_SHADER);
    lightShaderProgram.add("assets/shaders/default.geom", GL_GEOMETRY_SHADER);
    lightShaderProgram.link();

    Shader normalShaderProgram;
    normalShaderProgram.add("assets/shaders/default.vert", GL_VERTEX_SHADER);
    normalShaderProgram.add("assets/shaders/normal.frag", GL_FRAGMENT_SHADER);
    normalShaderProgram.add("assets/shaders/normal.geom", GL_GEOMETRY_SHADER);
    normalShaderProgram.link();

    GLuint VBO, VAO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVert.size() * sizeof(GLfloat), sphereVert.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIdx.size() * sizeof(GLuint), sphereIdx.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    GLuint groundVBO, groundVAO, groundEBO;
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);
    glGenVertexArrays(1, &groundVAO);

    glBindVertexArray(groundVAO);

    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, groundVert.size() * sizeof(GLfloat), groundVert.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, groundIdx.size() * sizeof(GLuint), groundIdx.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("assets/textures/wall.jpg", &width, &height, &nrChannels, 0);

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    glfwSetCursorPosCallback(window, mouse_callback);  
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // ============ FRAME COUNTER ============
    int frameCount = 0;
    double currentTime = glfwGetTime();
    double timeAccumulator = 0.0;
    // =======================================

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    int widthFramebuffer, heightFramebuffer;
    
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    lightModel = glm::translate(lightModel, lightPos);
    lightModel = glm::scale(lightModel, glm::vec3(0.2f, 0.2f, 0.2f));
    groundModel = glm::translate(groundModel, groundPos);
    groundModel = glm::scale(groundModel, glm::vec3(10.0f, 1.0f, 10.0f));

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glfwGetFramebufferSize(window, &widthFramebuffer, &heightFramebuffer);
        
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        proj = glm::perspective(glm::radians(fov), (float)widthFramebuffer / (float)heightFramebuffer, 0.001f, 100.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, widthFramebuffer, heightFramebuffer);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shaderProgram.use();
        shaderProgram.setInt("Texture", 0);
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("proj", proj);
        shaderProgram.setMat4("model", model);
        shaderProgram.setVec3("lightPos", lightPos);
        shaderProgram.setVec3("viewPos", cameraPos);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIdx.size()), GL_UNSIGNED_INT, 0);

        shaderProgram.setMat4("model", groundModel);
        glBindVertexArray(groundVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(groundIdx.size()), GL_UNSIGNED_INT, 0);

        lightShaderProgram.use();
        lightShaderProgram.setMat4("model", lightModel);
        lightShaderProgram.setMat4("view", view);
        lightShaderProgram.setMat4("proj", proj);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIdx.size()), GL_UNSIGNED_INT, 0);

        if (showNormals) {
            normalShaderProgram.use();
            normalShaderProgram.setMat4("model", model);
            normalShaderProgram.setMat4("view", view);
            normalShaderProgram.setMat4("proj", proj);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIdx.size()), GL_UNSIGNED_INT, 0);

            normalShaderProgram.setMat4("model", groundModel);
            glBindVertexArray(groundVAO);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(groundIdx.size()), GL_UNSIGNED_INT, 0);

            normalShaderProgram.setMat4("model", lightModel);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIdx.size()), GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        // ============ FRAME COUNTER ============
        frameCount++;
        currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        timeAccumulator += deltaTime;
        if (timeAccumulator >= 1.0) {
            std::string title = "LearnOpenGL - FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            timeAccumulator -= 1.0;
        }
        lastTime = currentTime;
        // =======================================
    }  
  
    glfwTerminate();
    return 0;
}