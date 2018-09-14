#ifndef GTMESHGRID_H
#define GTMESHGRID_H

#include "gtmeshbase.h"

class GtMeshGrid : public GtMeshBase
{
public:
    GtMeshGrid(qint32 width, qint32 height, qint32 sections);

    qint32 GetWidth() const { return _width; }
    qint32 GetHeight() const { return _height; }
    qint32 GetSectionsCount() const { return _sections; }
private:
    bool buildMesh() Q_DECL_OVERRIDE;
    void bindVAO(OpenGLFunctions* functions) Q_DECL_OVERRIDE;

protected:
    qint32 _width;
    qint32 _height;
    qint32 _sections;
};

#endif // GTMESHGRID_H
