#include "gtscene.h"
#include "Objects/gtobjectbase.h"

GtScene::GtScene()
{

}

void GtScene::addObject(GtObjectActor* object)
{
    objects.append(object);
}
