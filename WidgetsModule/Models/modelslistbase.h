#ifndef MODELSLISTBASE_H
#define MODELSLISTBASE_H

#include <QAbstractListModel>

#include "modelstablebase.h"

template<class T>
class TModelsListBase : public ModelsTableBase
{
    using Super = ModelsTableBase;
    using ValueExtractor = std::function<QVariant (const SharedPointer<T>&, const QModelIndex&, int)>;
    using CountExtractor = std::function<qint32 (const SharedPointer<T>&)>;
public:
    TModelsListBase(QObject* parent, const ValueExtractor& extractor, const CountExtractor& countExtractor)
        : Super(parent)
        , m_extractor(extractor)
        , m_countExtractor(countExtractor)
    {}
    TModelsListBase(QObject* parent)
        : Super(parent)
        , m_extractor([](const SharedPointer<T>&, const QModelIndex&, int){ return QVariant(); })
        , m_countExtractor([](const SharedPointer<T>&){ return 0; })
    {}

    void SetData(const SharedPointer<T>& data) { Super::SetData(data); }
    const SharedPointer<T>& GetData() const { return Super::GetData().template Cast<T>(); }

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& index = QModelIndex()) const override
    {
        Q_UNUSED(index);
        if(GetData() == nullptr) {
            return 0;
        }
        return m_countExtractor(GetData());
    }

    int columnCount(const QModelIndex& index = QModelIndex()) const override
    {
        Q_UNUSED(index);
        return 1;
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if(!index.isValid() || GetData() == nullptr) {
            return QVariant();
        }

        return m_extractor(GetData(), index, role);
    }

private:
    ValueExtractor m_extractor;
    CountExtractor m_countExtractor;
};

#endif // MODELSLISTBASE_H
