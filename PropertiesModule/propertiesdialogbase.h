#ifndef PROPERTIESDIALOGBASE_H
#define PROPERTIESDIALOGBASE_H

#ifdef QT_GUI_LIB

#include <QDialog>

#include "propertypromise.h"

/*
 * PropertiesDialogBase provides logic for working with properties by ui.
 * See also: PropertiesDialog
*/

class _Export PropertiesDialogBase : public QDialog
{
    typedef QDialog Super;
protected:
    typedef std::function<void ()> StdHandle;
    typedef std::function<void (qint32)> OnDoneHandle;

public:
    enum Option
    {
        Options_Default = 0x0,
        Option_ClearContextOnDone = 0x1,
        Option_ReadOnly = 0x2,
    };
    DECL_FLAGS(Options, Option)

    PropertiesDialogBase(const QString& name, const Name& scope, QWidget* view, QWidget* parent = nullptr);
    ~PropertiesDialogBase();

    static void RejectAllDialogs();
    static void CreateGeometryProperty(const QString& dialogName);
    void Initialize(const StdHandle& propertiesInitializeFunction = []{});
    const Options& GetOptions() const { return m_options; }
    Options& ChangeOptions() { return  m_options; }
    bool IsDirty() const { return !m_oldValues.isEmpty(); }
    void SetOnDone(const OnDoneHandle& onDone);
    template<class T> T* GetView() const { return reinterpret_cast<T*>(m_view); }

    // QDialog interface
public slots:
    virtual void done(int) Q_DECL_OVERRIDE;

    // QWidget interface
protected:
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

protected:
    virtual void changeProperties(const StdHandle& changingProperties);

protected:
    bool m_isInitialized;
    Options m_options;
    PropertiesScopeName m_scope;
    QWidget* m_view;
    ByteArrayPropertyPtr m_savedGeometry;
    QMetaObject::Connection m_connection;
    QHash<Property*, QVariant> m_oldValues;
    DispatcherConnectionsSafe m_additonalPropertiesConnections;

    static QSet<PropertiesDialogBase*>& currentDialogs();
};

#endif // QT_GUI_LIB

#endif // PROPERTIESDIALOGBASE_H
