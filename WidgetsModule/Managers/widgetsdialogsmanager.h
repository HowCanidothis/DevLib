#ifndef WIDGETSDIALOGSMANAGER_H
#define WIDGETSDIALOGSMANAGER_H

#include <QDialog>
#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

class WidgetsDialogsManager : public SingletoneGlobal<WidgetsDialogsManager>
{
    template<class T> friend class SingletoneGlobal;
    WidgetsDialogsManager();
public:
    QList<QString> AutomatedSourcePaths;

    void SetDefaultParentWindow(QWidget* window);
    QWidget* GetParentWindow() const;

    bool ShowSaveCancelDialog(const QString& label, const QString& text);
    bool ShowOkCancelDialog(const QString& label, const QString& text, const QString& confirmActionText = QString());
    void ShowMessageBox(QtMsgType msgType, const QString& title, const QString& message);
    QString GetText(const QString& title, const QString& text = QString(), bool* ok = nullptr);

    QDialog* GetOrCreateCustomDialog(const Name& tag, const std::function<DescCustomDialogParams ()>& paramsCreator);

    template<class T>
    T* GetOrCreateDialog(const Name& tag, const std::function<T* ()>& dialogCreator, const Name& restoreGeometryName = Name())
    {
        Q_ASSERT(!tag.IsNull());
        auto foundIt = m_taggedDialog.find(tag);
        if(foundIt != m_taggedDialog.end()) {
            return reinterpret_cast<T*>(foundIt.value());
        }
        auto* result = dialogCreator();
        initDialog(result, restoreGeometryName);
        m_taggedDialog.insert(tag, result);
        return result;
    }

    template<class T>
    T* GetOrCreateDialog(const Name& tag, const Name& restoreGeometryName = Name())
    {
        return GetOrCreateDialog<T>(tag, [this]{ return new T(GetParentWindow()); }, restoreGeometryName);
    }

    template<class T>
    T* CustomDialogView(QDialog* dialog) const
    {
        return (T*)(dialog->property(CustomViewPropertyKey).toLongLong());
    }
    void ShowDialog(QDialog* dialog, const DescShowDialogParams& params);
    void ShowPropertiesDialog(const PropertiesScopeName& name, const DescShowDialogParams& params);

    void ResizeDialogToDefaults(QWidget* dialog);
    void MakeFrameless(QWidget* widget, bool attachMovePane = true, const QString& movePaneId = QString());
    static void AttachShadow(class QWidget* w, bool applyMargins = true);

    QList<QUrl> SelectDirectory(const DescImportExportSourceParams& params);

    LocalPropertyDouble ShadowBlurRadius;
    LocalPropertyColor ShadowColor;

    CommonDispatcher<QWidget*> OnDialogCreated;
    static const char* CustomViewPropertyKey;

private:
    void initDialog(QWidget* w, const Name& restoreGeometryName);

private:
    QHash<Name, QWidget*> m_taggedDialog;
    QWidget* m_defaultParent;

    Q_DECLARE_TR_FUNCTIONS(WidgetsDialogsManager)
};

template<>
inline QList<ImportExportSourcePtr> ImportExportSource::CreateSources<WidgetsDialogsManager>(const DescImportExportSourceParams& params)
{
    if(!WidgetsDialogsManager::GetInstance().AutomatedSourcePaths.isEmpty()) {
        return ContainerBuilder<QList<ImportExportSourcePtr>>().Append(WidgetsDialogsManager::GetInstance().AutomatedSourcePaths, [](const auto& it){
            return ImportExportFileSource::FromFilePath(*it);
        });
    }
    return ContainerBuilder<QList<ImportExportSourcePtr>>().Append(lq::select(WidgetsDialogsManager::GetInstance().SelectDirectory(params),0), [](const auto& it){
        return ::make_shared<ImportExportFileSource>(*it);
    });
}

#endif // WIDGETSDIALOGMANAGER_H
