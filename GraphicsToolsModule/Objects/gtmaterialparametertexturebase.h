#ifndef GTMATERIALTEXTUREBASE_H
#define GTMATERIALTEXTUREBASE_H

#include "gtmaterialparameterbase.h"

class GtMaterialParameterTextureBase : public GtMaterialResourceParameterBase
{
    using Super = GtMaterialResourceParameterBase;
protected:
    gTexUnit m_unit;
public:
    GtMaterialParameterTextureBase(const QString& m_name, const Name& m_resource);

    // GtMaterialBase interface
protected:
    void updateTextureUnit(gTexUnit&) Q_DECL_OVERRIDE;
};

#endif // GTMATERIALTEXTUREBASE_H
