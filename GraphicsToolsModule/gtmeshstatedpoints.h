#ifndef GTMESHSTATEDPOINTS_H
#define GTMESHSTATEDPOINTS_H

#include "gtmeshbase.h"

class GtMeshStatedPoints : public GtMeshBase
{
public:
    GtMeshStatedPoints();

    void SetPoints(const QVector<StatedVertex3F>& points);

protected:
    bool buildMesh() override;
    void bindVAO(OpenGLFunctions *) override;

private:
    QVector<StatedVertex3F> m_points;
};

#endif // GTMESHSTATEDPOINTS_H
