#ifndef WIDGETSDIALOGSMANAGER_H
#define WIDGETSDIALOGSMANAGER_H


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

    QList<QUrl> SelectDirectory(const QString& dialogHeader, bool isSaveMode, const QString& fileName, const QStringList& filters);

    CommonDispatcher<QWidget*> OnDialogCreated;

private:
    QHash<Name, QWidget*> m_taggedDialog;
};

#endif // WIDGETSDIALOGMANAGER_H
