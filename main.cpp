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

// ������ɫ����Ƭ����ɫ������һ���ԣ�ֱ�ӳ���ɷ���
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
    // ����shader����
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        // ����length��Ϊ�������޷���char[length]�������飬���������ڴ�ķ�ʽ����,ע���ͷ��ڴ�
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
    // ���ӳ���
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

    // ʹ�ú��������ļ������Ǽ��������ļ�
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
    
    // ����֡��
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
        // ʹ�ú��������ļ�ʱ��Ҫ�ֶ�����vertexArrays����
        unsigned int vao;
        GLERRORCALL(glGenVertexArrays(1, &vao));
        GLERRORCALL(glBindVertexArray(vao));

        VertexBuffer vb(positions, vertexLength);
        // ���ö�������
        GLERRORCALL(glEnableVertexAttribArray(0));
        // gl�޷����Buffer�ڴ沼��
        // �ڼ�������,һ�������������ݵĸ���,�����������ݵ�����,�Ƿ��һ��0-255->0-1,һ��������ڴ��С,��Ԫ���ڶ����ڲ���ָ��ƫ��
        // ���·���ָ�����������ڶ������ݵ�����Ϊ0�������Ե�Ԫ������Ϊ2��Ԫ���е�������float��Ԫ�ص��ڴ��С��2*sizeof(float)����Ԫ���ڶ����ڲ���ƫ����0
        GLERRORCALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

        // ����Ҫ���ƾ���ʱ,��Ҫ�������������㣬��������ûᵼ�´�����gpu�ڴ��˷�
        // ��������
        IndexBuffer ib(indices, idxLength);

        ShaderProgrammerSource source = ParseShader("Basic.shader");
        unsigned int shader = CreateShader(source.vertexShader, source.fragmentShader);
        GLERRORCALL(glUseProgram(shader));

        // ��ȡͳһ����
        GLERRORCALL(int location = glGetUniformLocation(shader, "u_Color"));
        // ��ʹ������u_Color�����ڱ������ܻᵼ��u_Color������(����������δʹ��)
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
            // ����ͳһ����
            // ÿ�λ��Ƶ��ã��Ҳ�������������������Σ������ܿ������������εĲ�ͬ��ɫ����Ϊ��������һ�λ����У�
            GLERRORCALL(glUniform4f(location, r, 0.3f, 0.3f, 1.0f));
            glBindVertexArray(vao);
            ib.Bind();
            // �ɰ���Ʒ�ʽ:��ʱģʽ
            /*  glBegin(GL_TRIANGLES);
            glVertex2f(-0.5f, -0.5f);
            glVertex2f(-0.5f,  0.5f);
            glVertex2f(0.5f,   0.5f);
            glEnd();*/
            // �°���Ʒ�ʽ:����ģʽ
            // ���û��ƣ��ڴ�֮ǰ����CPU�ڹ���
            // glDrawArrays(GL_TRIANGLES, 0, 3);
            // ��������������
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