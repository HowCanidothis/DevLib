#include "modelserrorscomponents.h"

DescModelsErrorComponentAttachToErrorsContainerParameters::DescModelsErrorComponentAttachToErrorsContainerParameters(const Name& errorName, const TranslatedStringPtr& label, qint64 flags)
    : ErrorFlags(flags)
    , ErrorName(errorName)
    , Label(label)
{}

LocalPropertyErrorsModelWithErrorComponentMapper::LocalPropertyErrorsModelWithErrorComponentMapper(const char* cdl, class LocalPropertyErrorsModel* model, const LocalPropertyInt64& state)
    : m_model(model)
    , m_state(state)
    , m_cdl(cdl)
{

}

const LocalPropertyErrorsModelWithErrorComponentMapper& LocalPropertyErrorsModelWithErrorComponentMapper::Map(qint64 errorFlag, const Name& errorId) const
{
    m_model->Register(m_cdl, errorId, [errorFlag](auto state){
        return state & errorFlag;
    }, m_state);
    return *this;
}
