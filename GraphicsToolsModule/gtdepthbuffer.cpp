#include "gtdepthbuffer.h"

#include "gtframebufferobject.h"

GtDepthBuffer::GtDepthBuffer(OpenGLFunctions* f)
    : m_f(f)
{

}

void GtDepthBuffer::SetFrameBuffer(GtFramebufferObjectBase* frameBuffer, QOpenGLContext* context)
{
    m_frameBuffer = frameBuffer;
    m_context = context;
}

float GtDepthBuffer::ValueAt(qint32 x, qint32 y)
{
    m_context->makeCurrent(m_context->surface());
    m_frameBuffer->BindRead();
    float value;
    m_f->glReadPixels(x,m_frameBuffer->GetHeight() - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &value);
    m_frameBuffer->Release();

    return value;
}
