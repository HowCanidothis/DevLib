#include "modelserrorscomponents.h"

DescModelsErrorComponentAttachToErrorsContainerParameters::DescModelsErrorComponentAttachToErrorsContainerParameters(const Name& errorName, const TranslatedStringPtr& label, qint32 flags)
    : ErrorFlags(flags)
    , ErrorName(errorName)
    , Label(label)
{}
