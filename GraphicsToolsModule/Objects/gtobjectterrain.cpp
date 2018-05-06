#include "gtobjectterrain.h"
#include "../gtmeshsurface.h"

GtObjectTerrain::GtObjectTerrain()
{

}

void GtObjectTerrain::initialize(OpenGLFunctions* f)
{
    Q_ASSERT(surface_mesh.data());
    surface_mesh = new GtMeshSurface(3000, 2400, 320);
    surface_mesh->initialize(f);
}
