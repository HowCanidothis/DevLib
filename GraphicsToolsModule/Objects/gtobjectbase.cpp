#include "gtobjectbase.h"

bool GtActor::updateBoundingBox(BoundingBox& result)
{
    bool ret = false;
    for(GtPrimitiveActor* actor : m_components) {
        if(actor->updateBoundingBox()) {
            result.Unite(actor->GetBoundingBox());
            ret = true;
        }
    }
    return ret;
}
