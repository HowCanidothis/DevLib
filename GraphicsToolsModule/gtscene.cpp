#include "gtscene.h"
#include "Objects/gtobjectbase.h"

GtScene::GtScene()
{

}

GtScene::~GtScene()
{

}

void GtScene::AddObject(GtActor* object)
{
    m_objects.Append(object);
}
