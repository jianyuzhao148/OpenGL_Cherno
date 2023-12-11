#pragma once

#include <GL/glew.h>

// ������ͨ�����Ż�����
#define ASSERT(x) if(!(x)) __debugbreak();
#define GLERRORCALL(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x,__FILE__,__LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

