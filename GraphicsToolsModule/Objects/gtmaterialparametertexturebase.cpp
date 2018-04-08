#include "gtmaterialparametertexturebase.h"
#include "gtmaterial.h"

GtMaterialParameterTextureBase::GtMaterialParameterTextureBase(const QString& name, const QString& resource)
    : GtMaterialParameterBase(name, resource)
{

}

void GtMaterialParameterTextureBase::updateTextureUnit(gTexUnit& unit)
{
    this->unit = unit++;
}
