#include "VertexBuffer.h"
#include "Renderer.h"
#include "IndexBuffer.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
    GLERRORCALL(glGenBuffers(1, &m_RendererID));
    GLERRORCALL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
    GLERRORCALL(glBufferData(GL_ARRAY_BUFFER, size* sizeof(float), data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
    GLERRORCALL(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
    GLERRORCALL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::UnBind() const
{
    GLERRORCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
