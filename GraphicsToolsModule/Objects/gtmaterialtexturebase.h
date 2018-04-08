#ifndef GTMATERIALTEXTUREBASE_H
#define GTMATERIALTEXTUREBASE_H

#include "gtmaterialbase.h"

class GtMaterialTextureBase : public GtMaterialBase
{
protected:
    gTexUnit unit;
public:
    GtMaterialTextureBase(const QString& name, const QString& resource);

    // GtMaterialBase interface
protected:
    void updateTextureUnit(gTexUnit&) Q_DECL_OVERRIDE;
};

#endif // GTMATERIALTEXTUREBASE_H
