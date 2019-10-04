#include "gtobjectterrain.h"
#include "../gtmeshsurface.h"

GtObjectTerrain::GtObjectTerrain()
{

}

void GtObjectTerrain::Initialize(OpenGLFunctions* f)
{
    Q_ASSERT(m_surfaceMesh.data());
    m_surfaceMesh = new GtMeshSurface(3000, 2400, 320);
    m_surfaceMesh->Initialize(f);
}
