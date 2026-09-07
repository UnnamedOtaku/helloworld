#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <algorithm>
#include <array>
#include <iostream>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Shader.h"
#include "Light.h"
#include "Material.h"
#include "Texture.h"

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 groundModel = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 proj;

glm::vec3 groundPos(0.0f, -1.0f, 0.0f);
glm::vec3 spherePosition(0.0f);
glm::vec3 sphereScale(1.0f);
glm::vec3 groundScale(10.0f, 1.0f, 10.0f);

bool sphereExists = true;
bool groundExists = true;

enum class SceneElementType
{
    None,
    Sphere,
    Ground,
    Light
};

float deltaTime = 0.0f;
double lastTime = 0.0;

bool firstMouse = true;
bool cameraControlEnabled = false;
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX = 400, lastY = 300;
float fov   =  45.0f;

float tessellationLevel = 1.0f;

bool showNormals = false;
bool sceneInputEnabled = false;

GLuint sceneFramebuffer = 0;
GLuint sceneColorTexture = 0;
GLuint sceneDepthBuffer = 0;
int sceneTextureWidth = 0;
int sceneTextureHeight = 0;

void resizeSceneFramebuffer(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;

    if (sceneFramebuffer == 0)
    {
        glGenFramebuffers(1, &sceneFramebuffer);
        glGenTextures(1, &sceneColorTexture);
        glGenRenderbuffers(1, &sceneDepthBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer);
        glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTexture, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthBuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepthBuffer);
    }

    if (width == sceneTextureWidth && height == sceneTextureHeight)
        return;

    sceneTextureWidth = width;
    sceneTextureHeight = height;

    glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Scene framebuffer is incomplete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!cameraControlEnabled)
        return;

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse && !sceneInputEnabled)
        return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT &&
        (action == GLFW_PRESS || action == GLFW_RELEASE))
    {
        cameraControlEnabled = action == GLFW_PRESS;
        firstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR,
            cameraControlEnabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse && !sceneInputEnabled)
        return;

    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard && !sceneInputEnabled)
        return;

    if (key == GLFW_KEY_N && action == GLFW_PRESS)
        showNormals = !showNormals;

    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        tessellationLevel += 1.0f;
        if (tessellationLevel > 64.0f)
            tessellationLevel = 64.0f;
    }

    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        tessellationLevel -= 1.0f;
        if (tessellationLevel < 1.0f)
            tessellationLevel = 1.0f;
    }
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard && !sceneInputEnabled)
        return;

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

void drawSceneElements(
    std::vector<Light>& lights,
    bool& sphereExists,
    bool& groundExists,
    SceneElementType& selectedElementType,
    int& selectedElementIndex
)
{
    ImGui::Begin("Scene Elements");

    if (sphereExists)
    {
        bool sphereSelected = selectedElementType == SceneElementType::Sphere;
        if (ImGui::Selectable("Sphere", sphereSelected))
        {
            selectedElementType = SceneElementType::Sphere;
            selectedElementIndex = -1;
        }
        if (ImGui::BeginPopupContextItem("SphereContext"))
        {
            if (ImGui::MenuItem("Properties"))
            {
                selectedElementType = SceneElementType::Sphere;
                selectedElementIndex = -1;
            }
            if (ImGui::MenuItem("Delete"))
            {
                sphereExists = false;
                if (selectedElementType == SceneElementType::Sphere)
                    selectedElementType = SceneElementType::None;
            }
            ImGui::EndPopup();
        }
    }

    if (groundExists)
    {
        bool groundSelected = selectedElementType == SceneElementType::Ground;
        if (ImGui::Selectable("Ground", groundSelected))
        {
            selectedElementType = SceneElementType::Ground;
            selectedElementIndex = -1;
        }
        if (ImGui::BeginPopupContextItem("GroundContext"))
        {
            if (ImGui::MenuItem("Properties"))
            {
                selectedElementType = SceneElementType::Ground;
                selectedElementIndex = -1;
            }
            if (ImGui::MenuItem("Delete"))
            {
                groundExists = false;
                if (selectedElementType == SceneElementType::Ground)
                    selectedElementType = SceneElementType::None;
            }
            ImGui::EndPopup();
        }
    }

    for (int i = 0; i < static_cast<int>(lights.size()); ++i)
    {
        bool lightSelected = selectedElementType == SceneElementType::Light && selectedElementIndex == i;
        std::string label = "Light " + std::to_string(i + 1);
        if (ImGui::Selectable(label.c_str(), lightSelected))
        {
            selectedElementType = SceneElementType::Light;
            selectedElementIndex = i;
        }

        bool deleteLight = false;
        if (ImGui::BeginPopupContextItem("LightContext"))
        {
            if (ImGui::MenuItem("Properties"))
            {
                selectedElementType = SceneElementType::Light;
                selectedElementIndex = i;
            }
            deleteLight = ImGui::MenuItem("Delete");
            ImGui::EndPopup();
        }

        if (deleteLight)
        {
            lights.erase(lights.begin() + i);
            if (selectedElementType == SceneElementType::Light)
            {
                if (selectedElementIndex == i || lights.empty())
                    selectedElementType = SceneElementType::None;
                else if (selectedElementIndex > i)
                    --selectedElementIndex;
            }
            break;
        }
    }

    if (ImGui::BeginPopupContextWindow("SceneElementsContext", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::MenuItem("Light"))
            {
                lights.emplace_back(
                    LightType::POINT,
                    glm::vec3(0.0f, 2.0f, 0.0f),
                    glm::vec3(0.0f),
                    12.5f,
                    17.5f,
                    glm::vec3(51.0f),
                    glm::vec3(255.0f),
                    glm::vec3(255.0f),
                    1.0f,
                    0.045f,
                    0.0075f
                );
                selectedElementType = SceneElementType::Light;
                selectedElementIndex = static_cast<int>(lights.size()) - 1;
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void drawSelectedElementProperties(
    std::vector<Light>& lights,
    SceneElementType selectedElementType,
    int selectedElementIndex
)
{
    if (selectedElementType == SceneElementType::None)
        return;

    ImGui::Begin("Element Properties");

    if (selectedElementType == SceneElementType::Sphere)
        ImGui::TextUnformatted("Sphere");
    else if (selectedElementType == SceneElementType::Ground)
        ImGui::TextUnformatted("Ground");
    else if (selectedElementType == SceneElementType::Light)
        ImGui::Text("Light %d", selectedElementIndex + 1);

    if (selectedElementType == SceneElementType::Sphere)
    {
        ImGui::DragFloat3("Position", &spherePosition.x, 0.05f);
        ImGui::DragFloat3("Scale", &sphereScale.x, 0.05f, 0.01f, 100.0f);
        ImGui::DragFloat("Tessellation", &tessellationLevel, 0.25f, 1.0f, 64.0f);
    }
    else if (selectedElementType == SceneElementType::Ground)
    {
        ImGui::DragFloat3("Position", &groundPos.x, 0.05f);
        ImGui::DragFloat3("Scale", &groundScale.x, 0.05f, 0.01f, 100.0f);
    }
    else if (selectedElementType == SceneElementType::Light &&
             selectedElementIndex >= 0 && selectedElementIndex < static_cast<int>(lights.size()))
    {
        Light& light = lights[selectedElementIndex];
        ImGui::Checkbox("Enabled", &light.enabled);

        const char* lightTypes[] = { "Directional", "Point", "Spot" };
        int type = static_cast<int>(light.type);
        if (ImGui::Combo("Type", &type, lightTypes, IM_COUNTOF(lightTypes)))
            light.type = static_cast<LightType>(type);

        ImGui::DragFloat3("Position", &light.position.x, 0.05f);
        ImGui::DragFloat3("Target", &light.target.x, 0.05f);

        float innerCutoff = glm::degrees(glm::acos(glm::clamp(light.cutOff, -1.0f, 1.0f)));
        float outerCutoff = glm::degrees(glm::acos(glm::clamp(light.outerCutOff, -1.0f, 1.0f)));
        if (ImGui::DragFloat("Inner cutoff", &innerCutoff, 0.25f, 0.0f, 90.0f))
            light.cutOff = glm::cos(glm::radians(innerCutoff));
        if (ImGui::DragFloat("Outer cutoff", &outerCutoff, 0.25f, 0.0f, 90.0f))
            light.outerCutOff = glm::cos(glm::radians(outerCutoff));

        glm::vec3 ambient = light.ambient / 255.0f;
        glm::vec3 diffuse = light.diffuse / 255.0f;
        glm::vec3 specular = light.specular / 255.0f;
        if (ImGui::ColorEdit3("Ambient", &ambient.x))
            light.ambient = ambient * 255.0f;
        if (ImGui::ColorEdit3("Diffuse", &diffuse.x))
            light.diffuse = diffuse * 255.0f;
        if (ImGui::ColorEdit3("Specular", &specular.x))
            light.specular = specular * 255.0f;

        ImGui::DragFloat("Constant", &light.constant, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Linear", &light.linear, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Quadratic", &light.quadratic, 0.001f, 0.0f, 1.0f);
    }

    ImGui::End();
}

void initializeDockLayout(ImGuiID dockspaceId)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

    ImGuiID leftNode = 0;
    ImGuiID centerNode = 0;
    ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.20f, &leftNode, &centerNode);

    ImGuiID rightNode = 0;
    ImGui::DockBuilderSplitNode(centerNode, ImGuiDir_Right, 0.25f, &rightNode, &centerNode);

    ImGui::DockBuilderDockWindow("Scene Elements", leftNode);
    ImGui::DockBuilderDockWindow("Scene", centerNode);
    ImGui::DockBuilderDockWindow("Element Properties", rightNode);
    ImGui::DockBuilderFinish(dockspaceId);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
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

    generateSphereSmooth(1.0f, 12, 8, sphereVert, sphereIdx);

    Shader shaderProgram;
    shaderProgram.add("assets/shaders/default.vert", GL_VERTEX_SHADER);
    shaderProgram.add("assets/shaders/default.frag", GL_FRAGMENT_SHADER);
    shaderProgram.add("assets/shaders/default.geom", GL_GEOMETRY_SHADER);
    shaderProgram.link();

    Shader sphereShaderProgram;
    sphereShaderProgram.add("assets/shaders/sphere.vert", GL_VERTEX_SHADER);
    sphereShaderProgram.add("assets/shaders/sphere.tesc", GL_TESS_CONTROL_SHADER);
    sphereShaderProgram.add("assets/shaders/sphere.tese", GL_TESS_EVALUATION_SHADER);
    sphereShaderProgram.add("assets/shaders/default.frag", GL_FRAGMENT_SHADER);
    sphereShaderProgram.link();

    Shader lightShaderProgram;
    lightShaderProgram.add("assets/shaders/sphere.vert", GL_VERTEX_SHADER);
    lightShaderProgram.add("assets/shaders/sphere.tesc", GL_TESS_CONTROL_SHADER);
    lightShaderProgram.add("assets/shaders/sphere.tese", GL_TESS_EVALUATION_SHADER);
    lightShaderProgram.add("assets/shaders/light.frag", GL_FRAGMENT_SHADER);
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

    Texture diffuseTexture("assets/textures/diffuse_0.png", GL_TEXTURE_2D, 0);
    Texture specularTexture("assets/textures/specular_0.png", GL_TEXTURE_2D, 1);
    // Texture emissionTexture("assets/textures/emission_1.png", GL_TEXTURE_2D, 2);

    GLuint diffuseMap = diffuseTexture.id;
    GLuint specularMap = specularTexture.id;
    // GLuint emissionMap = emissionTexture.id;

    proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);  
    glfwSetMouseButtonCallback(window, mouse_button_callback);
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
    
    model = glm::translate(glm::mat4(1.0f), spherePosition);
    model = glm::scale(model, sphereScale);
    groundModel = glm::translate(glm::mat4(1.0f), groundPos);
    groundModel = glm::scale(groundModel, groundScale);
    
    Material material(0, 1, 2, 64.0f);

    sphereShaderProgram.use();
    sphereShaderProgram.setFloat("radius", 1.0f);
    material.Update(sphereShaderProgram);
    diffuseTexture.Update(sphereShaderProgram, "material.diffuse");
    specularTexture.Update(sphereShaderProgram, "material.specular");
    // emissionTexture.Update(sphereShaderProgram, "material.emission");

    shaderProgram.use();
    material.Update(shaderProgram);
    diffuseTexture.Update(shaderProgram, "material.diffuse");
    specularTexture.Update(shaderProgram, "material.specular");
    // emissionTexture.Update(shaderProgram, "material.emission");

    lightShaderProgram.use();
    lightShaderProgram.setFloat("radius", 1.0f);
    lightShaderProgram.setFloat("tessellationLevel", 1.0f);

    std::vector<Light> lights;
    lights.emplace_back(
        LightType::SPOT,
        glm::vec3(1.2f, 5.0f, -2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        12.5f, 17.5f,
        glm::vec3(51.0f),
        glm::vec3(127.5f),
        glm::vec3(255.0f),
        1.0f, 0.045f, 0.0075f
    );

    GLuint lightSSBO;
    glGenBuffers(1, &lightSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 440");

    SceneElementType selectedElementType = SceneElementType::None;
    int selectedElementIndex = -1;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glfwGetFramebufferSize(window, &widthFramebuffer, &heightFramebuffer);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, widthFramebuffer, heightFramebuffer);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(
            0,
            nullptr,
            ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking
        );
        static bool dockLayoutInitialized = false;
        if (!dockLayoutInitialized)
        {
            initializeDockLayout(dockspaceId);
            dockLayoutInitialized = true;
        }

        drawSceneElements(lights, sphereExists, groundExists, selectedElementType, selectedElementIndex);
        drawSelectedElementProperties(lights, selectedElementType, selectedElementIndex);

        model = glm::translate(glm::mat4(1.0f), spherePosition);
        model = glm::scale(model, sphereScale);
        groundModel = glm::translate(glm::mat4(1.0f), groundPos);
        groundModel = glm::scale(groundModel, groundScale);

        bool sceneWindowVisible = ImGui::Begin("Scene");
        sceneInputEnabled = sceneWindowVisible && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        ImVec2 sceneSize = ImGui::GetContentRegionAvail();
        int requestedSceneWidth = static_cast<int>(sceneSize.x);
        int requestedSceneHeight = static_cast<int>(sceneSize.y);

        if (sceneWindowVisible && requestedSceneWidth > 0 && requestedSceneHeight > 0)
        {
            resizeSceneFramebuffer(requestedSceneWidth, requestedSceneHeight);

            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            proj = glm::perspective(
                glm::radians(fov),
                static_cast<float>(sceneTextureWidth) / static_cast<float>(sceneTextureHeight),
                0.001f,
                100.0f
            );

            glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer);
            glViewport(0, 0, sceneTextureWidth, sceneTextureHeight);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        sphereShaderProgram.use();
        sphereShaderProgram.setMat4("view", view);
        sphereShaderProgram.setMat4("proj", proj);
        sphereShaderProgram.setMat4("model", model);
        sphereShaderProgram.setVec3("viewPos", cameraPos);
        sphereShaderProgram.setFloat("tessellationLevel", tessellationLevel);
        sphereShaderProgram.setInt("lightCount", static_cast<int>(lights.size()));

        std::vector<GpuLight> gpuLights;
        gpuLights.reserve(lights.size());
        for (const Light& light : lights)
            gpuLights.push_back(light.GetGpuData());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, gpuLights.size() * sizeof(GpuLight), gpuLights.data(), GL_DYNAMIC_DRAW);

        diffuseTexture.Bind();
        specularTexture.Bind();
        // emissionTexture.Bind();

        if (sphereExists)
        {
            glPatchParameteri(GL_PATCH_VERTICES, 3);
            glBindVertexArray(VAO);
            glDrawElements(GL_PATCHES, static_cast<GLsizei>(sphereIdx.size()), GL_UNSIGNED_INT, 0);
        }

        shaderProgram.use();
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("proj", proj);
        shaderProgram.setMat4("model", groundModel);
        shaderProgram.setVec3("viewPos", cameraPos);
        shaderProgram.setInt("lightCount", static_cast<int>(lights.size()));

        diffuseTexture.Bind();
        specularTexture.Bind();
        // emissionTexture.Bind();

        if (groundExists)
        {
            glBindVertexArray(groundVAO);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(groundIdx.size()), GL_UNSIGNED_INT, 0);
        }

        if (showNormals)
        {
            normalShaderProgram.use();
            normalShaderProgram.setMat4("model", model);
            normalShaderProgram.setMat4("view", view);
            normalShaderProgram.setMat4("proj", proj);

            if (sphereExists)
            {
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIdx.size()), GL_UNSIGNED_INT, 0);
            }

            normalShaderProgram.setMat4("model", groundModel);
            if (groundExists)
            {
                glBindVertexArray(groundVAO);
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(groundIdx.size()), GL_UNSIGNED_INT, 0);
            }
        }

        lightShaderProgram.use();
        lightShaderProgram.setMat4("view", view);
        lightShaderProgram.setMat4("proj", proj);
        lightShaderProgram.setFloat("tessellationLevel", 1.0f);
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        glBindVertexArray(VAO);
        for (const Light& light : lights)
        {
            glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), light.position);
            lightModel = glm::scale(lightModel, glm::vec3(0.12f));
            lightShaderProgram.setMat4("model", lightModel);
            lightShaderProgram.setVec3("lightColor", light.enabled ? light.diffuse / 255.0f : glm::vec3(0.25f));
            glDrawElements(GL_PATCHES, static_cast<GLsizei>(sphereIdx.size()), GL_UNSIGNED_INT, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, widthFramebuffer, heightFramebuffer);
        if (sceneWindowVisible && sceneTextureWidth > 0 && sceneTextureHeight > 0)
        {
            ImGui::Image(ImTextureRef(sceneColorTexture), sceneSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteFramebuffers(1, &sceneFramebuffer);
    glDeleteTextures(1, &sceneColorTexture);
    glDeleteRenderbuffers(1, &sceneDepthBuffer);
  
    glfwTerminate();
    return 0;
}