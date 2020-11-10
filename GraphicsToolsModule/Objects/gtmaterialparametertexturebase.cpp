#include "gtmaterialparametertexturebase.h"
#include "gtmaterial.h"

GtMaterialParameterTextureBase::GtMaterialParameterTextureBase(const QString& name, const Name& resource)
    : Super(name, resource)
{

}

void GtMaterialParameterTextureBase::updateTextureUnit(gTexUnit& unit)
{
    this->m_unit = unit++;
}
