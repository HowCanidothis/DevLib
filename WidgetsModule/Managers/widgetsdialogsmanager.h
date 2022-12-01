#ifndef WIDGETSDIALOGSMANAGER_H
#define WIDGETSDIALOGSMANAGER_H

#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

class WidgetsDialogsManager : public SingletoneGlobal<WidgetsDialogsManager>
{
    template<class T> friend class SingletoneGlobal;
    WidgetsDialogsManager();
public:
    void SetDefaultParentWindow(QWidget* window);
    QWidget* GetParentWindow() const;

    bool ShowSaveCancelDialog(const QString& label, const QString& text);
    bool ShowOkCancelDialog(const QString& label, const QString& text);
    void ShowMessageBox(QtMsgType msgType, const QString& title, const QString& message);

    QDialog* GetOrCreateCustomDialog(const Name& tag, const std::function<DescCustomDialogParams ()>& paramsCreator);

    template<class T>
    T* GetOrCreateDialog(const Name& tag, const std::function<T* ()>& dialogCreator, const Name& restoreGeometryName = Name())
    {
        auto foundIt = m_taggedDialog.find(tag);
        if(foundIt != m_taggedDialog.end()) {
            return reinterpret_cast<T*>(foundIt.value());
        }
        auto* result = dialogCreator();
        OnDialogCreated(result);
        if(!restoreGeometryName.IsNull()) {
            QSettings geometriesSettings;
            auto geometry = geometriesSettings.value("Geometries/" + restoreGeometryName.AsString()).toByteArray();
            if(!geometry.isEmpty()) {
                result->restoreGeometry(geometry);
            }
            WidgetWrapper(result).AddEventFilter([result, restoreGeometryName](QObject*, QEvent* event){
                if(event->type() == QEvent::Hide) {
                    QSettings geometriesSettings;
                    geometriesSettings.setValue("Geometries/" + restoreGeometryName.AsString(), result->saveGeometry());
                }
                return false;
            });
        }
        m_taggedDialog.insert(tag, result);
        return result;
    }

    template<class T>
    T* CustomDialogView(QDialog* dialog) const
    {
        return (T*)(dialog->property(CustomViewPropertyKey).toLongLong());
    }
    void ShowDialog(QDialog* dialog, const DescShowDialogParams& params);
    void ShowPropertiesDialog(const PropertiesScopeName& name, const DescShowDialogParams& params);

    void ResizeDialogToDefaults(QWidget* dialog);
    void MakeFrameless(QWidget* widget, bool attachMovePane = true);
    static void AttachShadow(class QWidget* w, bool applyMargins = true);

    QList<QUrl> SelectDirectory(const DescImportExportSourceParams& params);

    LocalPropertyDouble ShadowBlurRadius;
    LocalPropertyColor ShadowColor;

    CommonDispatcher<QWidget*> OnDialogCreated;
    static const char* CustomViewPropertyKey;

private:
    QHash<Name, QWidget*> m_taggedDialog;
    QWidget* m_defaultParent;

    Q_DECLARE_TR_FUNCTIONS(WidgetsDialogsManager)
};

template<>
inline QList<ImportExportSourcePtr> ImportExportSource::CreateSources<WidgetsDialogsManager>(const DescImportExportSourceParams& params)
{
    return ContainerBuilder<QList<ImportExportSourcePtr>>().Append(lq::select(WidgetsDialogsManager::GetInstance().SelectDirectory(params),0), [](const auto& it){
        return ::make_shared<ImportExportFileSource>(*it);
    });
}

#endif // WIDGETSDIALOGMANAGER_H
