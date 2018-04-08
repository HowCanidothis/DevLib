#ifndef GTMATERIALTEXTUREBASE_H
#define GTMATERIALTEXTUREBASE_H

#include "gtmaterialparameterbase.h"

class GtMaterialParameterTextureBase : public GtMaterialParameterBase
{
protected:
    gTexUnit unit;
public:
    GtMaterialParameterTextureBase(const QString& name, const QString& resource);

    // GtMaterialBase interface
protected:
    void updateTextureUnit(gTexUnit&) Q_DECL_OVERRIDE;
};

#endif // GTMATERIALTEXTUREBASE_H
