#include "gtdepthbuffer.h"

#include "gtframebufferobject.h"

GtDepthBuffer::GtDepthBuffer(OpenGLFunctions* f)
    : m_f(f)
{

}

void GtDepthBuffer::SetFrameBuffer(GtFramebufferObjectBase* frameBuffer)
{
    m_frameBuffer = frameBuffer;
}

float GtDepthBuffer::ValueAt(qint32 x, qint32 y)
{
    m_frameBuffer->Bind();
    float value;
    m_f->glReadPixels(x,m_frameBuffer->GetHeight() - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &value);
    m_frameBuffer->Release();

    return value;
}
