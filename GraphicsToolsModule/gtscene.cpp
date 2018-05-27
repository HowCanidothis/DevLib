#include "gtscene.h"
#include "Objects/gtobjectbase.h"

GtScene::GtScene()
{

}

GtScene::~GtScene()
{

}

void GtScene::addObject(GtActor* object)
{
    objects.Append(object);
}
