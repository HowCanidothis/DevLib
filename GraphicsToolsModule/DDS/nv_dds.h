#pragma once

#include <string>
#include <deque>
#include <istream>

#include <assert.h>
#include <stdint.h>

#include "SharedGuiModule/decl.h"

#ifndef NV_DDS_NO_GL_SUPPORT
    #ifdef __APPLE__
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#endif

namespace nv_dds {
enum TextureType {
    TextureNone, TextureFlat,    // 1D, 2D textures
    Texture3D,
    TextureCubemap
};

#ifdef NV_DDS_NO_GL_SUPPORT
    #define GL_RGB                            0x1907
    #define GL_RGBA                           0x1908
    #define GL_LUMINANCE                      0x1909
    #define GL_BGR_EXT                        0x80E0
    #define GL_BGRA_EXT                       0x80E1
#endif

class DDSSurface {
public:
    DDSSurface();
    DDSSurface(unsigned int w, unsigned int h, unsigned int d, unsigned int imgsize, const uint8_t* pixels);
    DDSSurface(const DDSSurface& copy);
    DDSSurface& operator=(const DDSSurface& rhs);
    virtual ~DDSSurface();

    operator uint8_t*() const;

    virtual void Create(unsigned int w, unsigned int h, unsigned int d, unsigned int imgsize, const uint8_t* pixels);
    virtual void Clear();

    unsigned int GetWidth() const {
        return m_width;
    }
    unsigned int GetHeight() const {
        return m_height;
    }
    unsigned int GetDepth() const {
        return m_depth;
    }
    unsigned int GetSize() const {
        return m_size;
    }

private:
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_depth;
    unsigned int m_size;

    uint8_t* m_pixels;
};

class DDSTexture: public DDSSurface {
    friend class DDSImage;

public:
    DDSTexture();
    DDSTexture(unsigned int w, unsigned int h, unsigned int d, unsigned int imgsize, const uint8_t* pixels);
    DDSTexture(const DDSTexture& copy);
    DDSTexture& operator=(const DDSTexture& rhs);
    ~DDSTexture();

    void Create(unsigned int w, unsigned int h, unsigned int d, unsigned int imgsize, const uint8_t* pixels);
    void Clear();

    const DDSSurface& GetMipmap(unsigned int index) const {
        assert(!m_mipmaps.empty());
        assert(index < m_mipmaps.size());

        return m_mipmaps[index];
    }

    void AddMipmap(const DDSSurface &mipmap) {
        m_mipmaps.push_back(mipmap);
    }

    unsigned int GetNumMipmaps() const {
        return (unsigned int) m_mipmaps.size();
    }

protected:
    DDSSurface& GetMipmap(unsigned int index) {
        assert(!m_mipmaps.empty());
        assert(index < m_mipmaps.size());

        return m_mipmaps[index];
    }

private:
    std::deque<DDSSurface> m_mipmaps;
};

class DDSImage {
public:
    DDSImage(OpenGLFunctions* f);
    ~DDSImage();

    void CreateTextureFlat(unsigned int format, unsigned int components, const DDSTexture &baseImage);
    void CreateTexture3D(unsigned int format, unsigned int components, const DDSTexture &baseImage);
    void CreateTextureCubemap(unsigned int format, unsigned int components, const DDSTexture &positiveX, const DDSTexture &negativeX, const DDSTexture &positiveY,
            const DDSTexture &negativeY, const DDSTexture &positiveZ, const DDSTexture &negativeZ);

    void Clear();

    void Load(std::istream& is, bool flipImage = true);
    void Load(const std::string& filename, bool flipImage = true);
    void Save(const std::string& filename, bool flipImage = true);

#ifndef NV_DDS_NO_GL_SUPPORT
#if !defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
    void UploadTexture1D();
#endif

    void UploadTexture2D(uint32_t imageIndex = 0, uint32_t target = GL_TEXTURE_2D);

#ifndef GL_ES_VERSION_2_0
    void UploadTexture3D();
#endif

    void UploadTextureCubemap();
#endif

    operator uint8_t*() {
        assert(m_valid);
        assert(!m_images.empty());

        return m_images[0];
    }

    unsigned int GetWidth() {
        assert(m_valid);
        assert(!m_images.empty());

        return m_images[0].GetWidth();
    }

    unsigned int GetHeight() {
        assert(m_valid);
        assert(!m_images.empty());

        return m_images[0].GetHeight();
    }

    unsigned int GetDepth() {
        assert(m_valid);
        assert(!m_images.empty());

        return m_images[0].GetDepth();
    }

    unsigned int GetSize() {
        assert(m_valid);
        assert(!m_images.empty());

        return m_images[0].GetSize();
    }

    unsigned int GetNumMipmaps() {
        assert(m_valid);
        assert(!m_images.empty());

        return m_images[0].GetNumMipmaps();
    }

    const DDSSurface& GetMipmap(unsigned int index) const {
        assert(m_valid);
        assert(!m_images.empty());
        assert(index < m_images[0].GetNumMipmaps());

        return m_images[0].GetMipmap(index);
    }

    const DDSTexture &GetCubemapFace(unsigned int face) const {
        assert(m_valid);
        assert(!m_images.empty());
        assert(m_images.size() == 6);
        assert(m_type == TextureCubemap);
        assert(face < 6);

        return m_images[face];
    }

    unsigned int GetComponents() {
        return m_components;
    }
    unsigned int GetFormat() {
        return m_format;
    }
    TextureType GetType() {
        return m_type;
    }

    bool IsCompressed();

    bool IsCubemap() {
        return (m_type == TextureCubemap);
    }
    bool IsVolume() {
        return (m_type == Texture3D);
    }
    bool IsValid() {
        return m_valid;
    }

    bool IsDwordAligned() {
        assert(m_valid);

        int dwordLineSize = getDwordAlignedLinesize(GetWidth(), m_components * 8);
        int curLineSize = GetWidth() * m_components;

        return (dwordLineSize == curLineSize);
    }

private:
    unsigned int clampSize(unsigned int size);
    unsigned int sizeDxtc(unsigned int width, unsigned int height);
    unsigned int sizeRgb(unsigned int width, unsigned int height);

    // calculates 4-byte aligned width of image
    unsigned int getDwordAlignedLinesize(unsigned int width, unsigned int bpp) {
        return ((width * bpp + 31) & -32) >> 3;
    }

    void flip(DDSSurface &surface);
    void flipTexture(DDSTexture &texture);

    void writeTexture(const DDSTexture &texture, std::ostream& os);

    unsigned int m_format;
    unsigned int m_components;
    TextureType m_type;
    bool m_valid;

    std::deque<DDSTexture> m_images;

    OpenGLFunctions* f;
};
}
