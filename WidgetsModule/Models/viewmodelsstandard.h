#ifndef VIEWMODELSSTANDARD_H
#define VIEWMODELSSTANDARD_H

#include "viewmodelsdefaultcomponents.h"

class ModelsStandardRow : public QVector<QVariant>
{
    using Super = QVector<QVariant>;
public:
    using Super::Super;

    void Set(qint32 i, const QVariant& data);
    QVariant Get(qint32 i) const;
    bool IsValid(qint32 i) const;

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
        buffer << buffer.Sect("Rows", *reinterpret_cast<Super*>(this));
    }
};

DECLARE_MODEL_BY_TYPE(ModelsStandardRow);

class ViewModelsStandardComponentsBuilder : protected TViewModelsColumnComponentsBuilder<ModelsStandardRowModel>
{
    using Super = TViewModelsColumnComponentsBuilder<ModelsStandardRowModel>;
public:
    using Super::Super;

    void AddColumn(qint32 i, const FTranslationHandler& title);
    void SetColumn(qint32 i, const FTranslationHandler& title);
    void SetColumnsCount(qint32 i);
};

class ViewModelsStandard : public TViewModelsTableBase<ModelsStandardRowModel>
{
    using Super = TViewModelsTableBase<ModelsStandardRowModel>;
public:
    ViewModelsStandard(QObject* parent);

// QAbstractItemModel interface
public:
    QVariant data(const QModelIndex& index, int role) const override;
};

#endif // VIEWMODELSSTANDARD_H