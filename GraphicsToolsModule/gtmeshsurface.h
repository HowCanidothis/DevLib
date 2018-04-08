#ifndef GTMESHSURFACE_H
#define GTMESHSURFACE_H

#include "gtmeshbase.h"

class GtMeshSurface : public GtMeshIndicesBase
{
public:
    GtMeshSurface(qint32 width, qint32 height, qint32 sections);
    ~GtMeshSurface();

    qint32 getWidth() const { return width; }
    qint32 getHeight() const { return height; }
    qint32 getSectionsCount() const { return sections; }
private:
    bool buildMesh() Q_DECL_OVERRIDE;
    void bindVAO(OpenGLFunctions* functions) Q_DECL_OVERRIDE;
private:
    qint32 width;
    qint32 height;
    qint32 sections;
};

#endif // SURFACEMESH_H
