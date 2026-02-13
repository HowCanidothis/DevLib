#ifndef VIEWMODELSSTANDARD_H
#define VIEWMODELSSTANDARD_H

#include "viewmodelsdefaultcomponents.h"

class ModelsStandardRow : public QVector<QHash<qint32,QVariant>>
{
    using Super = QVector<QHash<qint32,QVariant>>;
public:
    using Super::Super;

    template<typename ... Args>
    ModelsStandardRow(const Args&... variants)
        : Super(sizeof...(variants))
    {
        qint32 i = 0;
        adapters::Combine([&](const auto& v) {
            (*this)[i++][Qt::DisplayRole] = v;
        }, variants...);
    }

    void Set(qint32 i, const QVariant& data, qint32 role = Qt::DisplayRole);
    QVariant Get(qint32 i, qint32 role) const;
    bool IsValid(qint32 i) const;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << buffer.Sect("Rows", *reinterpret_cast<Super*>(this));
    }
};

DECLARE_MODEL_BY_TYPE(ModelsStandardRow);

class ViewModelsStandardComponentsBuilder : public TViewModelsColumnComponentsBuilder<ModelsStandardRowModel>
{
    using Super = TViewModelsColumnComponentsBuilder<ModelsStandardRowModel>;
public:
    using Super::Super;

    ViewModelsStandardComponentsBuilder& AddColumn(qint32 i, const FTranslationHandler& title);
    ViewModelsStandardComponentsBuilder& SetColumn(qint32 i, const FTranslationHandler& title);
    ViewModelsStandardComponentsBuilder& SetColumnsCount(qint32 i);
};

class ViewModelsStandard : public TViewModelsTableBase<ModelsStandardRowModel>
{
    using Super = TViewModelsTableBase<ModelsStandardRowModel>;
public:
    ViewModelsStandard(QObject* parent);

    ViewModelsStandardComponentsBuilder Builder();

// QAbstractItemModel interface
public:
    bool setData(const QModelIndex& index, const QVariant& v, qint32 role) override;
    QVariant data(const QModelIndex& index, int role) const override;
};

#endif // VIEWMODELSSTANDARD_H
