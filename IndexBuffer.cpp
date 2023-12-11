#include "IndexBuffer.h"
#include "Renderer.h"

 IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
     :m_Count(count)
{
    GLERRORCALL(glGenBuffers(1, &m_RendererID));
    GLERRORCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
    GLERRORCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count* sizeof(unsigned int), data, GL_STATIC_DRAW));
}

 IndexBuffer::~IndexBuffer()
{
    GLERRORCALL(glDeleteBuffers(1, &m_RendererID));
}

void IndexBuffer::Bind() const
{
    GLERRORCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
}

void IndexBuffer::UnBind() const
{
    GLERRORCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
