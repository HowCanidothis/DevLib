#ifndef VIEWMODELVACABULARY_H
#define VIEWMODELVACABULARY_H

#include "modelstablebase.h"

class ModelsVocabularyViewModel : public TViewModelsTableBase<ModelsVocabulary>
{
    using Super = TViewModelsTableBase<ModelsVocabulary>;
public:
    ModelsVocabularyViewModel(QObject* parent = nullptr);

    bool setData(const QModelIndex& index, const QVariant& value, qint32 role) override;
    QVariant data(const QModelIndex& index, qint32 role) const override;
    qint32 rowCount(const QModelIndex&) const override;
    qint32 columnCount(const QModelIndex&) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<qint32, std::function<void (QVariant&)>> GetterDisplayDelegates;
    QHash<qint32, std::function<void (QVariant&)>> GetterDelegates;
    QHash<qint32, std::function<void (QVariant&)>> SetterDelegates;
};


#endif // VIEWMODELVACABULARY_H
