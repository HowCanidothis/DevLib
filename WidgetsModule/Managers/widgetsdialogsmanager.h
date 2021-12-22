#ifndef WIDGETSDIALOGSMANAGER_H
#define WIDGETSDIALOGSMANAGER_H

#include <SharedModule/internal.hpp>

class WidgetsDialogsManager
{
    WidgetsDialogsManager();
public:
    static WidgetsDialogsManager& GetInstance();

    template<class T>
    T* ShowOrCreateDialog(const Name& tag, const std::function<T* ()>& dialogCreator)
    {
        auto foundIt = m_taggedDialog.find(tag);
        if(foundIt != m_taggedDialog.end()) {
            return reinterpret_cast<T*>(foundIt.value());
        }
        auto* result = dialogCreator();
        OnDialogCreated(result);
        m_taggedDialog.insert(tag, result);
        return result;
    }

    QList<QUrl> SelectDirectory(const DescImportExportSourceParams& params);

    CommonDispatcher<QWidget*> OnDialogCreated;

private:
    QHash<Name, QWidget*> m_taggedDialog;
};

template<>
inline QList<ImportExportSourcePtr> ImportExportSource::CreateSources<WidgetsDialogsManager>(const DescImportExportSourceParams& params)
{
    return lq::Select<ImportExportSourcePtr>(WidgetsDialogsManager::GetInstance().SelectDirectory(params), [](const QUrl& url){ return ::make_shared<ImportExportFileSource>(url); });
}

#endif // WIDGETSDIALOGMANAGER_H
