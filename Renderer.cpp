#include "Renderer.h"

#include <iostream>

void GLClearError() {
    // 取出error数据,直到没错误为止
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function,const char* file,int line)
{
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] " << error << " " << file << " " << function << " " << line << std::endl;
        return false;
    }
	return true;
}

