#ifndef GTMESHSURFACE_H
#define GTMESHSURFACE_H

#include "gtmeshbase.h"

class GtMeshSurface : public GtMeshIndices
{
public:
    GtMeshSurface(qint32 width, qint32 height, qint32 sections);
    ~GtMeshSurface();

    qint32 GetWidth() const { return m_width; }
    qint32 GetHeight() const { return m_height; }
    qint32 GetSectionsCount() const { return m_sections; }

private:
    void updateBuffer();

protected:
    qint32 m_width;
    qint32 m_height;
    qint32 m_sections;
};

#endif // SURFACEMESH_H
