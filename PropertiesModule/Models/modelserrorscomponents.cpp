#include "modelserrorscomponents.h"

DescModelsErrorComponentAttachToErrorsContainerParameters::DescModelsErrorComponentAttachToErrorsContainerParameters(const Name& errorName, const TranslatedStringPtr& label, qint64 flags)
    : ErrorFlags(flags)
    , ErrorName(errorName)
    , Label(label)
{}
