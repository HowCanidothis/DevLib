#include "gtrenderpath.h"

#include "gtscene.h"
#include "gtrenderer.h"
#include "Objects/gtmaterial.h"
#include "gtmeshquad2D.h"

/*
template<class T>
class GtStorageBuffer
{
public:
    GtStorageBuffer(quint32 bufferType)
        : m_handle(-1)
        , m_bufferType(bufferType)
    {}

    bool IsValid() const { return m_handle != -1; }
    void Initialize(OpenGLFunctions* f, quint32 usage, const T* initial = nullptr)
    {
        f->glGenBuffers(1, &m_handle);
        f->glBindBuffer(m_bufferType, m_handle);
        f->glBufferData(m_bufferType, sizeof(T), initial, usage);
    }

    void Bind(OpenGLFunctions* f)
    {
        f->glBindBuffer(m_bufferType, m_handle);
    }

    void Release(OpenGLFunctions* f)
    {
        f->glBindBuffer(m_bufferType, 0);
    }

    void* Map(OpenGLFunctions* f, quint32 offset, quint32 length, quint32 access)
    {
        glBindBuffer(m_bufferType, m_handle);
        return f->glMapBufferRange(m_bufferType, offset, length, access);
    }

    void UnMap(OpenGLFunctions* f)
    {
        f->glUnmapBuffer(m_bufferType);
    }

private:
    quint32 m_handle;
    quint32 m_bufferType;
};
*/

GtDefaultRenderPath::GtDefaultRenderPath(GtRenderer* renderer)
    : Super(renderer)
{

}

void GtDefaultRenderPath::Render(GtScene* scene, qint32)
{
    scene->DrawAll(f);
}

GtRenderPath::~GtRenderPath()
{

}

void GtRenderPath::disableDepthTest()
{
    m_renderer->disableDepthTest();
}

void GtRenderPath::enableDepthTest()
{
    m_renderer->enableDepthTest();
}

GtRenderPath::GtRenderPath(GtRenderer* f)
    : m_renderer(f)
    , f(f)
{

}
