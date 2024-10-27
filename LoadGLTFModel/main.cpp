#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <tiny_gltf.h>

const int TINYGLTF_MODE_DEFAULT = -1;
const int TINYGLTF_MODE_POINT = 0;
const int TINYGLTF_MODE_TRIANGLE = 4;
const int TINYGLTF_COMPONETTYPE_UNSHORT = 5123;
const int TINYGLTF_COMPONETTYPE_UNINT = 5125;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 顶点着色器源码
const char* pVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// 片段着色器源码
const char* pFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";

void processInput(GLFWwindow* vWindow)
{
    if (glfwGetKey(vWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(vWindow, true);
}

bool loadGLTF(const std::string& vFilename, tinygltf::Model& vModelGLTF)
{
    tinygltf::TinyGLTF Loader;
    std::string Err;
    std::string Warn;

    bool Res = Loader.LoadASCIIFromFile(&vModelGLTF, &Err, &Warn, vFilename);

    if (!Warn.empty())
    {
        std::cout << "WARN : " << Warn;
    }

    if (!Err.empty())
    {
        std::cout << "ERR : " << Err;
    }

    if (!Res)
    {
        std::cout << "Failed to load glTF : " << vFilename << std::endl;
    }
    else
    {
        std::cout << "Loaded glTF : " << vFilename << std::endl;
    }

    return Res;
}

void createIndiceBufferData(std::vector<unsigned int>& vIndices, const tinygltf::BufferView& vBufferView, const tinygltf::Buffer& vBuffer, const int& vComponentType)
{
    unsigned short TempUShortIndice;
    unsigned int   TempUIntIndice;
    const int UnShortByte = 2;
    const int UnIntByte = 4;
    if (vComponentType == TINYGLTF_COMPONETTYPE_UNSHORT)
    {
        for (size_t i = vBufferView.byteOffset; i < vBufferView.byteOffset + vBufferView.byteLength; i += UnShortByte)
        {
            std::memcpy(&TempUShortIndice, &vBuffer.data.at(i), sizeof(unsigned short));
            vIndices.push_back(TempUShortIndice);
        }
    }
    else if (vComponentType == TINYGLTF_COMPONETTYPE_UNINT)
    {
        for (size_t i = vBufferView.byteOffset; i < vBufferView.byteOffset + vBufferView.byteLength; i += UnIntByte)
        {
            std::memcpy(&TempUIntIndice, &vBuffer.data.at(i), sizeof(unsigned int));
            vIndices.push_back(TempUIntIndice);
        }
    }
}

void createVertexBufferData(std::vector<float>& vVertices, const tinygltf::Buffer& vBuffer, const int vIndex)
{
    float TempVertice;
    const int FloatByte = 4;
    const int FloatNum = 3;
    for (auto i = vIndex; i < vIndex + FloatNum * FloatByte; i += FloatByte)
    {
        std::memcpy(&TempVertice, &vBuffer.data.at(i), sizeof(float));
        vVertices.push_back(TempVertice);
    }
}

void createVerticeAndIndice(tinygltf::Model& vGLTFModel, std::vector<float>& vioVertices, std::vector<unsigned int>& vioIndices)
{
    for (auto& Node : vGLTFModel.nodes)
    {
        if (Node.mesh == -1) continue;
        const auto& Mesh = vGLTFModel.meshes[Node.mesh];
        std::string MeshName = Mesh.name;
        std::cout << "MeshName : " << MeshName << std::endl;

        for (auto& Primitive : Mesh.primitives)
        {
            vioVertices.clear();
            if (Primitive.mode == TINYGLTF_MODE_POINT)
            {
                const tinygltf::Accessor& AccessorPos = vGLTFModel.accessors[Primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& BufferViewPos = vGLTFModel.bufferViews[AccessorPos.bufferView];
                const tinygltf::Buffer& BufferPos = vGLTFModel.buffers[BufferViewPos.buffer];
                const tinygltf::Accessor& AccessorColor = vGLTFModel.accessors[Primitive.attributes.at("COLOR_0")];
                const tinygltf::BufferView& BufferViewColor = vGLTFModel.bufferViews[AccessorColor.bufferView];
                const tinygltf::Buffer& BufferColor = vGLTFModel.buffers[BufferViewColor.buffer];
                glm::vec3 MinPos(AccessorPos.minValues[0], AccessorPos.minValues[1], AccessorPos.minValues[2]);
                glm::vec3 MaxPos(AccessorPos.maxValues[0], AccessorPos.maxValues[1], AccessorPos.maxValues[2]);

                const int Vec3Byte = 12;
                for (size_t i = BufferViewPos.byteOffset, k = BufferViewColor.byteOffset;
                    (i < BufferViewPos.byteOffset + BufferViewPos.byteLength && k < BufferViewColor.byteOffset + BufferViewColor.byteLength);
                    i += Vec3Byte, k += Vec3Byte)
                {
                    createVertexBufferData(vioVertices, BufferPos, (int)i);
                    createVertexBufferData(vioVertices, BufferColor, (int)k);
                }

                std::cout << "Vertices.size : " << vioVertices.size() << std::endl;
                assert(vioVertices.size() == vGLTFModel.accessors[Primitive.attributes.at("POSITION")].count * 3 * 2);
            }
            else if (Primitive.mode == TINYGLTF_MODE_TRIANGLE || Primitive.mode == TINYGLTF_MODE_DEFAULT)
            {
                vioVertices.clear();
                vioIndices.clear();
                const tinygltf::BufferView& BufferViewIndice = vGLTFModel.bufferViews[vGLTFModel.accessors[Primitive.indices].bufferView];
                const tinygltf::Buffer& BufferIndice = vGLTFModel.buffers[BufferViewIndice.buffer];
                const int IndiceComponentType = vGLTFModel.accessors[Primitive.indices].componentType;

                createIndiceBufferData(vioIndices, BufferViewIndice, BufferIndice, IndiceComponentType);
                std::cout << "indice.size : " << vioIndices.size() << std::endl;
                assert(vioIndices.size() == vGLTFModel.accessors[Primitive.indices].count);

                const tinygltf::BufferView& BufferViewPos = vGLTFModel.bufferViews[vGLTFModel.accessors[Primitive.attributes.at("POSITION")].bufferView];
                const tinygltf::Buffer& BufferPos = vGLTFModel.buffers[BufferViewPos.buffer];
                const tinygltf::BufferView& BufferViewNor = vGLTFModel.bufferViews[vGLTFModel.accessors[Primitive.attributes.at("NORMAL")].bufferView];
                const tinygltf::Buffer& BufferNor = vGLTFModel.buffers[BufferViewNor.buffer];

                assert(BufferViewPos.byteLength == BufferViewNor.byteLength);

                const int Vec3Byte = 12;
                for (std::size_t i = BufferViewPos.byteOffset, k = BufferViewNor.byteOffset;
                    (i < BufferViewPos.byteOffset + BufferViewPos.byteLength && k < BufferViewNor.byteOffset + BufferViewNor.byteLength);
                    i += Vec3Byte, k += Vec3Byte)
                {
                    createVertexBufferData(vioVertices, BufferPos, (int)i);
                    createVertexBufferData(vioVertices, BufferNor, (int)k);
                }
                std::cout << "Vertices.size : " << vioVertices.size() << std::endl;
                assert(vioVertices.size() == vGLTFModel.accessors[Primitive.attributes.at("POSITION")].count * 6);
            }
        }
    }
    return;
}

int main()
{
    // 初始化GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 创建窗口
    GLFWwindow* pWindow = glfwCreateWindow(800, 600, "Colorful Square with Phong Lighting", nullptr, nullptr);
    if (!pWindow)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(pWindow);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 设置视口
    glViewport(0, 0, 800, 600);

    // 编译顶点着色器
    unsigned int VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &pVertexShaderSource, nullptr);
    glCompileShader(VertexShader);
    // 检查编译错误
    int Success;
    char InfoLog[512];
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        glGetShaderInfoLog(VertexShader, 512, nullptr, InfoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << InfoLog << std::endl;
    }

    // 编译片段着色器
    unsigned int FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &pFragmentShaderSource, nullptr);
    glCompileShader(FragmentShader);
    // 检查编译错误
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        glGetShaderInfoLog(FragmentShader, 512, nullptr, InfoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << InfoLog << std::endl;
    }

    // 链接着色器程序
    unsigned int ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);
    // 检查链接错误
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (!Success)
    {
        glGetProgramInfoLog(ShaderProgram, 512, nullptr, InfoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << InfoLog << std::endl;
    }

    // 删除着色器
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    tinygltf::Model GLTFModel;
    loadGLTF("./models/dragon.gltf", GLTFModel);
    std::vector<float> Vertices;
    std::vector<unsigned int> Indices;
    createVerticeAndIndice(GLTFModel, Vertices, Indices);

    unsigned int DragonVBO, DragonVAO, DragonEBO;
    glGenVertexArrays(1, &DragonVAO);
    glGenBuffers(1, &DragonVBO);
    glGenBuffers(1, &DragonEBO);

    glBindVertexArray(DragonVAO);

    glBindBuffer(GL_ARRAY_BUFFER, DragonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Vertices.size(), Vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DragonEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), Indices.data(), GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 法线属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 主循环
    while (!glfwWindowShouldClose(pWindow))
    {
        // 输入处理
        processInput(pWindow);

        // 渲染指令
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活着色器程序
        glUseProgram(ShaderProgram);

        // 设置变换矩阵
        glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -40.0f));
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.0f, 100.0f);
        unsigned int modelLoc = glGetUniformLocation(ShaderProgram, "model");
        unsigned int projLoc = glGetUniformLocation(ShaderProgram, "projection");
        unsigned int viewLoc = glGetUniformLocation(ShaderProgram, "view");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(View));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(Projection));

        // 设置光源属性
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        glUniform3fv(glGetUniformLocation(ShaderProgram, "lightPos"), 1, &lightPos[0]);
        glm::vec3 viewPos(0.0f, 0.0f, 3.0f);
        glUniform3fv(glGetUniformLocation(ShaderProgram, "viewPos"), 1, &viewPos[0]);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        glUniform3fv(glGetUniformLocation(ShaderProgram, "lightColor"), 1, &lightColor[0]);

        // 绘制正方形
        glBindVertexArray(DragonVAO);
        glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

        // 交换缓冲区
        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    // 释放资源
    glDeleteVertexArrays(1, &DragonVAO);
    glDeleteBuffers(1, &DragonVBO);
    glDeleteBuffers(1, &DragonEBO);
    glDeleteProgram(ShaderProgram);

    glfwTerminate();
    return 0;
}
