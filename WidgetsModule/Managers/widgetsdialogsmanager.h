#ifndef WIDGETSDIALOGSMANAGER_H
#define WIDGETSDIALOGSMANAGER_H

#include <QDialogButtonBox>

#include <PropertiesModule/internal.hpp>

class WidgetsDialogsManager : public SingletoneGlobal<WidgetsDialogsManager>
{
    template<class T> friend class SingletoneGlobal;
    WidgetsDialogsManager();
public:
    void SetDefaultParentWindow(QWidget* window);
    QWidget* GetParentWindow() const;

    bool ShowOkCancelDialog(const QString& label, const QString& text);
    void ShowMessageBox(QtMsgType msgType, const QString& title, const QString& message);

    struct DescCustomDialogParams
    {
        QVector<std::tuple<QDialogButtonBox::ButtonRole, QString, FAction>> Buttons;
        QWidget* View = nullptr;
        qint32 DefaultButtonIndex = 0;
        bool DefaultSpacing = true;

        DescCustomDialogParams& FillWithText(const QString& text);
        DescCustomDialogParams& SetDefaultSpacing(bool defaultSpacing) { DefaultSpacing = defaultSpacing; return *this; }
        DescCustomDialogParams& AddButton(QDialogButtonBox::ButtonRole role, const QString& text, const FAction& action)
        {
            Buttons.append(std::make_tuple(role, text, action));
            return *this;
        }
        DescCustomDialogParams& SetView(QWidget* view) { Q_ASSERT(View == nullptr); View = view; return *this; }
    };

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
            WidgetsAttachment::Attach(result, [result, restoreGeometryName](QObject*, QEvent* event){
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

    struct DescShowDialogParams
    {
        bool Modal = true;
        bool ResizeToDefault = false;

        DescShowDialogParams& SetModal(bool modal) { Modal = modal; return *this; }
        DescShowDialogParams& SetResizeToDefault(bool resize) { ResizeToDefault = resize; return *this; }
    };

    void ShowDialog(QDialog* dialog, const DescShowDialogParams& params);
    void ShowPropertiesDialog(const PropertiesScopeName& name, const DescShowDialogParams& params);

    void ResizeDialogToDefaults(QWidget* dialog);
    void MakeFrameless(QWidget* widget, bool attachMovePane = true);
    static void AttachShadow(class QWidget* w, bool applyMargins = true);

    QList<QUrl> SelectDirectory(const DescImportExportSourceParams& params);

    LocalPropertyDouble ShadowBlurRadius;
    LocalPropertyColor ShadowColor;

    CommonDispatcher<QWidget*> OnDialogCreated;

private:
    QHash<Name, QWidget*> m_taggedDialog;
    QWidget* m_defaultParent;

    Q_DECLARE_TR_FUNCTIONS(WidgetsDialogsManager)
};

template<>
inline QList<ImportExportSourcePtr> ImportExportSource::CreateSources<WidgetsDialogsManager>(const DescImportExportSourceParams& params)
{
    return lq::Select<ImportExportSourcePtr>(WidgetsDialogsManager::GetInstance().SelectDirectory(params), [](const QUrl& url){ return ::make_shared<ImportExportFileSource>(url); });
}

#endif // WIDGETSDIALOGMANAGER_H
