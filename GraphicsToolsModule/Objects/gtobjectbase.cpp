#include "gtobjectbase.h"

bool GtActor::updateBoundingBox(BoundingBox& result)
{
    bool ret = false;
    for(GtPrimitiveActor* actor : components) {
        if(actor->updateBoundingBox()) {
            result.Unite(actor->getBoundingBox());
            ret = true;
        }
    }
    return ret;
}
