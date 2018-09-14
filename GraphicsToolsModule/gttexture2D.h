#ifndef GTTEXTURE_H
#define GTTEXTURE_H
#include <SharedGuiModule/internal.hpp>
#include "ResourcesModule/internal.hpp"

struct GtTextureFormat
{
    quint32 min_filter = GL_NEAREST;
    quint32 mag_filter = GL_NEAREST;
    quint32 wrap_s = GL_CLAMP_TO_EDGE;
    quint32 wrap_t = GL_CLAMP_TO_EDGE;

    gPixFormat pixels_format = GL_RGBA;
    gPixType pixels_type = GL_FLOAT;
    const void* pixels = nullptr;
};
Q_DECLARE_TYPEINFO(GtTextureFormat, Q_PRIMITIVE_TYPE);

class GtTexture
{
public:
    GtTexture(OpenGLFunctions* f, gTexTarget target)
        : f(f)
        , texture_id(0)
        , internal_format(0)
        , target(target)
        , allocated(false)
    {}
    ~GtTexture();

    void setSize(quint32 w, quint32 h);
    void setInternalFormat(gTexInternalFormat internal_format);

    void bind();
    void bind(quint32 unit);

    void release();

    bool create();
    bool isCreated() const { return texture_id != 0; }
    bool isValid() const;

    const QSize& getSize() const { return size; }
    gTexTarget getTarget() const { return target; }
    gTexID getID() const { return texture_id; }

    virtual void allocate(const GtTextureFormat& format = GtTextureFormat())=0;
    static GtTexture* create(OpenGLFunctions* f, gTexTarget target, gTexInternalFormat internal_format, const SizeI& size, const GtTextureFormat* format);
protected:
    OpenGLFunctions* f;
    gTexID texture_id;
    gTexInternalFormat internal_format;
    gTexTarget target;
    bool allocated;
    QSize size;
};

class GtTexture2D : public GtTexture
{
public:
    GtTexture2D(OpenGLFunctions *f);

    void loadImage(const QString& img_file);
    void load(const QString& dds_file);
    static void bindTexture(OpenGLFunctions* f, gTexUnit unit, gTexID id);
    void allocate(const GtTextureFormat& format = GtTextureFormat()) Q_DECL_OVERRIDE;
private:
};

class GtTexture2DMultisampled : public GtTexture
{
public:
    GtTexture2DMultisampled(OpenGLFunctions* f, quint32 samples);

    void allocate(const GtTextureFormat& format = GtTextureFormat()) Q_DECL_OVERRIDE;
private:
    quint32 samples;
};

class GtTextureBinder
{
    GtTexture* texture;
public:
    explicit GtTextureBinder(GtTexture* texture) Q_DECL_NOEXCEPT
        : texture(texture)
    {
        texture->bind();
    }
    ~GtTextureBinder()
    {
        texture->release();
    }
};

#endif // GTTEXTURE_H
