#pragma once

#include <GL/glew.h>

// 错误处理通过宏优化代码
#define ASSERT(x) if(!(x)) __debugbreak();
#define GLERRORCALL(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x,__FILE__,__LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

