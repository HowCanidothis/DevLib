#ifndef VIEWMODELVACABULARY_H
#define VIEWMODELVACABULARY_H

#include "modelstablebase.h"

class ModelsVocabularyViewModel : public TViewModelsEditTable<TViewModelsTableBase<ModelsVocabulary>>
{
    using Super = TViewModelsEditTable<TViewModelsTableBase<ModelsVocabulary>>;
public:
    ModelsVocabularyViewModel(QObject* parent = nullptr);

    qint32 columnCount(const QModelIndex&) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<qint32, std::function<void (QVariant&)>> GetterDisplayDelegates;
    QHash<qint32, std::function<void (QVariant&)>> GetterDelegates;
    QHash<qint32, std::function<void (QVariant&)>> SetterDelegates;
};


#endif // VIEWMODELVACABULARY_H
