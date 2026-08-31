#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <iostream>

#include "Shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
} 

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void generateSphereSmooth(float radius, int sectors, int stacks,
                          std::vector<GLfloat>& vertices,
                          std::vector<GLuint>& indices) {
    
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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    std::vector<GLfloat> sphereVert;
    std::vector<GLuint> sphereIdx;

    generateSphereSmooth(1.0f, 20, 15, sphereVert, sphereIdx);

    Shader shaderProgram;
    shaderProgram.add("assets/shaders/default_vert.glsl", GL_VERTEX_SHADER);
    shaderProgram.add("assets/shaders/default_frag.glsl", GL_FRAGMENT_SHADER);
    shaderProgram.link();

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

    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    glm::mat4 proj;
    proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // ============ FRAME COUNTER ============
    int frameCount = 0;
    double lastTime = glfwGetTime();
    double currentTime;
    // =======================================

    glEnable(GL_DEPTH_TEST);

    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        model = glm::rotate(model, (float)glfwGetTime() * deltaTime * 0.1f * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f)); 

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shaderProgram.use();

        shaderProgram.setMat4("model", model);
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("proj", proj);

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, sphereIdx.size() * sizeof(GLuint), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // ============ FRAME COUNTER ============
        frameCount++;
        currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
            std::string title = "LearnOpenGL - FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, title.c_str());
            deltaTime = 1.0f / frameCount;
            frameCount = 0;
            lastTime = currentTime;
        }
        // =======================================
    }  
  
    glfwTerminate();
    return 0;
}