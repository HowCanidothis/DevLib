#ifndef WIDGETSDIALOGSMANAGER_H
#define WIDGETSDIALOGSMANAGER_H

#include <PropertiesModule/internal.hpp>

class WidgetsDialogsManager : public SingletoneGlobal<WidgetsDialogsManager>
{
    template<class T> friend class SingletoneGlobal;
    WidgetsDialogsManager();
public:
    void SetDefaultParentWindow(QWidget* window);
    QWidget* GetParentWindow() const;

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

    void MakeFrameless(QWidget* widget, bool attachMovePane = true);
    static void AttachShadow(class QWidget* w);

    QList<QUrl> SelectDirectory(const DescImportExportSourceParams& params);

    LocalPropertyDouble ShadowBlurRadius;
    LocalPropertyColor ShadowColor;

    CommonDispatcher<QWidget*> OnDialogCreated;

private:
    QHash<Name, QWidget*> m_taggedDialog;
    QWidget* m_defaultParent;
};

template<>
inline QList<ImportExportSourcePtr> ImportExportSource::CreateSources<WidgetsDialogsManager>(const DescImportExportSourceParams& params)
{
    return lq::Select<ImportExportSourcePtr>(WidgetsDialogsManager::GetInstance().SelectDirectory(params), [](const QUrl& url){ return ::make_shared<ImportExportFileSource>(url); });
}

#endif // WIDGETSDIALOGMANAGER_H
