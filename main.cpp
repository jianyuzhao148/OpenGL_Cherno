#include <GL/glew.h>l

#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"


struct ShaderProgrammerSource {
    std::string vertexShader;
    std::string fragmentShader;
};

static ShaderProgrammerSource ParseShader(const std::string& filePath)
{
    enum class ShaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT =1,
    };

    std::fstream stream(filePath);
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
            else if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
        }
        else
        {
            ss[(int)type] << line << "\n";
        }
    }
    return { ss[0].str(),ss[1].str() };
}

// 顶点着色器和片段着色器具有一致性，直接抽象成方法
static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    //const char* src = &source[0]//crazy
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // TODO: Error Handling 
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    // 编译shader出错
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        // 由于length不为常量，无法用char[length]声明数组，改用申请内存的方式处理,注意释放内存
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Faild compile" << (type== GL_VERTEX_SHADER?"vertex" : "fragment") << " shader" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        free(message);
        return 0;
    }

    return id;
}

static int CreateShader(const std::string& vertexShader, const std::string& framgmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER,vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, framgmentShader);
    // 附加程序
    glAttachShader(program, vs);
    glAttachShader(program,fs);
    glLinkProgram(program);
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // 使用核心配置文件，而非兼容配置文件
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    
    // 设置帧率
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        std::cout << "ERROR!" << std::endl;
    {
        // vertex data
        const unsigned int vertexLength = 8;
        float positions[vertexLength] = {
            -0.5f,-0.5f,
            -0.5f,0.5f,
            0.5f,0.5f,
            0.5f,-0.5f
        };

        // index data
        const unsigned int idxLength = 6;
        unsigned int indices[idxLength] = {
            0,2,3,
            0,1,2,
        };
        // 使用核心配置文件时需要手动创建vertexArrays对象
        unsigned int vao;
        GLERRORCALL(glGenVertexArrays(1, &vao));
        GLERRORCALL(glBindVertexArray(vao));

        VertexBuffer vb(positions, vertexLength);
        // 启用顶点属性
        GLERRORCALL(glEnableVertexAttribArray(0));
        // gl无法理解Buffer内存布局
        // 第几个属性,一个顶点所有数据的个数,顶点描述数据的类型,是否归一化0-255->0-1,一个顶点的内存大小,该元素在顶点内部的指针偏移
        // 以下方法指明：该属性在顶点数据的索引为0，该属性的元素数量为2，元素中的类型是float，元素的内存大小是2*sizeof(float)，该元素在顶点内部的偏移是0
        GLERRORCALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

        // 当需要绘制矩形时,需要复用三角形两点，如果不复用会导致大量的gpu内存浪费
        // 索引缓冲
        IndexBuffer ib(indices, idxLength);

        ShaderProgrammerSource source = ParseShader("Basic.shader");
        unsigned int shader = CreateShader(source.vertexShader, source.fragmentShader);
        GLERRORCALL(glUseProgram(shader));

        // 获取统一变量
        GLERRORCALL(int location = glGetUniformLocation(shader, "u_Color"));
        // 即使声明了u_Color但是在编译后可能会导致u_Color不存在(比如声明但未使用)
        ASSERT(location != -1);

        glBindVertexArray(0);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        float r = 0.0f;
        float increment = 0.03f;
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT);
            GLERRORCALL(glUseProgram(shader));
            // 创建统一变量
            // 每次绘制调用（找不到比如绘制两个三角形，并不能控制两个三角形的不同颜色，因为他们是在一次绘制中）
            GLERRORCALL(glUniform4f(location, r, 0.3f, 0.3f, 1.0f));
            glBindVertexArray(vao);
            ib.Bind();
            // 旧版绘制方式:即时模式
            /*  glBegin(GL_TRIANGLES);
            glVertex2f(-0.5f, -0.5f);
            glVertex2f(-0.5f,  0.5f);
            glVertex2f(0.5f,   0.5f);
            glEnd();*/
            // 新版绘制方式:核心模式
            // 调用绘制，在此之前都是CPU在工作
            // glDrawArrays(GL_TRIANGLES, 0, 3);
            // 绘制索引缓冲区
            GLERRORCALL(glDrawElements(GL_TRIANGLES, idxLength, GL_UNSIGNED_INT, nullptr));

            if (r > 1) {
                increment = -0.03f;
            }
            else if (r < 0) {
                increment = 0.03f;
            }
            r += increment;
            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }
    glfwTerminate();
    return 0;
}