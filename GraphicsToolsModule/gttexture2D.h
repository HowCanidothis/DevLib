#ifndef GTTEXTURE_H
#define GTTEXTURE_H
#include <SharedGuiModule/internal.hpp>
#include "ResourcesModule/internal.hpp"

struct GtTextureFormat
{
    quint32 MinFilter = GL_NEAREST;
    quint32 MagFilter = GL_NEAREST;
    quint32 WrapS = GL_CLAMP_TO_EDGE;
    quint32 WrapT = GL_CLAMP_TO_EDGE;

    gPixFormat PixelFormat = GL_RGBA;
    gPixType PixelType = GL_FLOAT;
    const void* Pixels = nullptr;
};
Q_DECLARE_TYPEINFO(GtTextureFormat, Q_PRIMITIVE_TYPE);

class GtTexture
{
public:
    GtTexture(OpenGLFunctions* f, gTexTarget target)
        : f(f)
        , m_textureId(0)
        , m_internalFormat(0)
        , m_target(target)
        , m_allocated(false)
    {}
    ~GtTexture();

    void SetSize(quint32 w, quint32 h);
    void SetInternalFormat(gTexInternalFormat m_internalFormat);

    void Bind();
    void Bind(quint32 unit);

    void Release();

    bool Create();
    bool IsCreated() const { return m_textureId != 0; }
    bool IsValid() const;

    const QSize& GetSize() const { return m_size; }
    gTexTarget GetTarget() const { return m_target; }
    gTexID GetId() const { return m_textureId; }

    virtual void Allocate(const GtTextureFormat& format = GtTextureFormat())=0;
    static GtTexture* Create(OpenGLFunctions* f, gTexTarget m_target, gTexInternalFormat m_internalFormat, const SizeI& m_size, const GtTextureFormat* format);

protected:
    OpenGLFunctions* f;
    gTexID m_textureId;
    gTexInternalFormat m_internalFormat;
    gTexTarget m_target;
    bool m_allocated;
    QSize m_size;
};

class GtTexture2D : public GtTexture
{
public:
    GtTexture2D(OpenGLFunctions *f);

    void LoadImg(const QString& img_file);
    void Load(const QString& dds_file);
    static void bindTexture(OpenGLFunctions* f, gTexUnit unit, gTexID id);
    void Allocate(const GtTextureFormat& format = GtTextureFormat()) Q_DECL_OVERRIDE;

private:
};

class GtTexture2DMultisampled : public GtTexture
{
public:
    GtTexture2DMultisampled(OpenGLFunctions* f, quint32 m_samples);

    void Allocate(const GtTextureFormat& format = GtTextureFormat()) Q_DECL_OVERRIDE;

private:
    quint32 m_samples;
};

class GtTextureBinder
{
    GtTexture* m_texture;
public:
    explicit GtTextureBinder(GtTexture* texture) Q_DECL_NOEXCEPT
        : m_texture(texture)
    {
        texture->Bind();
    }
    ~GtTextureBinder()
    {
        m_texture->Release();
    }
};

#endif // GTTEXTURE_H
