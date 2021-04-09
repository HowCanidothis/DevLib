#include "gtdepthbuffer.h"

#include "gtframebufferobject.h"

GtDepthBuffer::GtDepthBuffer(OpenGLFunctions* f)
    : m_f(f)
    , m_context(nullptr)
    , m_frameBuffer(nullptr)
{

}

void GtDepthBuffer::SetFrameBuffer(GtFramebufferObjectBase* frameBuffer, QOpenGLContext* context)
{
    m_frameBuffer = frameBuffer;
    m_context = context;
}

float GtDepthBuffer::ValueAt(qint32 x, qint32 y)
{
    if(m_frameBuffer == nullptr || m_context == nullptr) {
        return 0.f;
    }
    m_context->makeCurrent(m_context->surface());
    m_frameBuffer->BindRead();
    float value[81];
    m_f->glReadPixels(x - 5,m_frameBuffer->GetHeight() - (y - 5), 9, 9, GL_DEPTH_COMPONENT, GL_FLOAT, &value);
    m_frameBuffer->Release();
    return *std::min_element(std::begin(value), std::end(value));
}
