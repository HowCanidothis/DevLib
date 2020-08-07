#ifndef MODELSTABLEBASE_H
#define MODELSTABLEBASE_H

#include <QAbstractTableModel>

#include "wrappers.h"

class ModelsTableBase : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
public:
    ModelsTableBase(QObject* parent);
    ~ModelsTableBase();

    void SetData(const ModelsTableWrapperPtr& data);
    const ModelsTableWrapperPtr& GetData() const { return m_data; }

private:
    ModelsTableWrapperPtr m_data;
};

template<class T>
class TModelsTableBase : public ModelsTableBase
{
    using Super = ModelsTableBase;
public:
    using Super::Super;

    void SetData(const SharedPointer<T>& data) { Super::SetData(data); }
    const SharedPointer<T>& GetData() const { return Super::GetData().template Cast<T>(); }
};

#endif // MODELSTABLEBASE_H
