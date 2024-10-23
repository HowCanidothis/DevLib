#ifndef WIDGETSDIALOGSMANAGER_H
#define WIDGETSDIALOGSMANAGER_H

#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

class WidgetsDialogsManager : public SingletoneGlobal<WidgetsDialogsManager>
{
    template<class T> friend class SingletoneGlobal;
    WidgetsDialogsManager();
public:
    static const char* FDialogHandlerPropertyName;
    static const char* ResizeablePropertyName;
    static const char* CustomViewPropertyKey;

    using FDialogHandler = std::function<void (class WidgetsDialog*)>;
    QList<QString> AutomatedSourcePaths;

    void SetDefaultParentWindow(QWidget* window);
    QWidget* GetParentWindow() const;

    bool ShowDeleteCancelDialog(const QString& title, const QString& text);
    qint32 ShowTempDialog(const DescCustomDialogParams& params, const DescShowDialogParams& showParams = DescShowDialogParams()) const;
    void ShowTextDialog(const QString& title, const QString& text) const;
    std::optional<QString> GetText(const FTranslationHandler& title, const QString& text = QString(), const QStringList& keys = QStringList());
    std::optional<QColor> GetColor(const QColor& color = QColor(), bool showAlpha = false);
    std::optional<QDate> GetDate(const FTranslationHandler& title, const QDate& current = QDate());
    std::optional<std::pair<QDate, QDate>> GetDateRange(const FTranslationHandler& title, const QDate& from = QDate(), const QDate& to = QDate());
    QList<QUrl> SelectDirectory(const DescImportExportSourceParams& params);

    template<class T>
    T* GetOrCreateDialog(const Name& tag, const Name& restoreGeometryName = Name())
    {
        return GetOrCreateDialog<T>(tag, [this]{ return new T(GetParentWindow()); }, restoreGeometryName);
    }

    template<class T>
    T* GetOrCreateDialog(const Name& tag, const std::function<T* ()>& dialogCreator, const Name& restoreGeometryName = Name())
    {
        Q_ASSERT(!tag.IsNull());
        auto foundIt = m_taggedDialog.find(tag);
        if(foundIt != m_taggedDialog.end()) {
            Q_ASSERT(qobject_cast<T*>(foundIt.value()));
            return reinterpret_cast<T*>(foundIt.value());
        }
        auto* result = dialogCreator();
        OnDialogCreated(result);
        if(result->parentWidget() == nullptr) {
            result->setParent(GetParentWindow(), result->windowFlags());
        }
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

    WidgetsDialog* GetOrCreateDialog(const Name& tag, const std::function<DescCustomDialogParams ()>& paramsCreator);
    qint32 ShowDialog(WidgetsDialog* dialog, const DescShowDialogParams& params = DescShowDialogParams()) const;

    void ResizeDialogToDefaults(QWidget* dialog) const;
    void MakeFrameless(QWidget* widget, bool attachMovePane = true, const QString& movePaneId = QString());
    static void AttachShadow(class QWidget* w, bool applyMargins = true);

    LocalPropertyDouble ShadowBlurRadius;
    LocalPropertyColor ShadowColor;

    CommonDispatcher<QWidget*> OnDialogCreated;

private:
    WidgetsDialog* createDialog(const DescCustomDialogParams& params) const;

private:
    QHash<Name, QDialog*> m_taggedDialog;
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
