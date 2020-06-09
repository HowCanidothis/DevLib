#ifndef GTPOINTSMESH_H
#define GTPOINTSMESH_H

#include "gtmeshbase.h"

class GtPointsMesh : public GtMeshBase
{
    using Super = GtMeshBase;
public:
    GtPointsMesh();

    void SetPoints(const QVector<Point3F>& points);
    void SetPoints(const QVector<ColoredVertex3F>& points);

    // GtMeshBase interface
protected:
    bool buildMesh() override;
    void bindVAO(OpenGLFunctions*) override;

private:
    QVector<ColoredVertex3F> m_points;

    // GtMeshBase interface
public:
};

#endif // GTPOINTSMESH_H
