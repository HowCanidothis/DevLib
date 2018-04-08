#include "gtmaterialtexturebase.h"
#include "gtviewbase.h"

GtMaterialTextureBase::GtMaterialTextureBase(const QString& name, const QString& resource)
    : GtMaterialBase(name, resource)
{

}

void GtMaterialTextureBase::updateTextureUnit(gTexUnit& unit)
{
    this->unit = unit++;
}
