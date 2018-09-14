#ifndef PROPERTIESDIALOGBASE_H
#define PROPERTIESDIALOGBASE_H

#include <QDialog>

#include "propertypromise.h"

class _Export PropertiesDialogBase : public QDialog
{
    typedef QDialog Super;
protected:
    typedef std::function<void ()> StdHandle;
    typedef std::function<void (qint32)> OnDoneHandle;

public:
    PropertiesDialogBase(const QString& name, qint32 contextIndex, QWidget* view, QWidget* parent = nullptr, Qt::WindowFlags flags= Qt::WindowFlags());

    void Initialize(const StdHandle& propertiesInitializeFunction, const StdHandle& onChange, bool isReadOnly);
    void SetOnDone(const OnDoneHandle& onDone);
    bool IsDirty() const { return _isDirty; }
    template<class T> T* GetView() const { return reinterpret_cast<T*>(_view); }

    // QDialog interface
public slots:
    virtual void done(int) Q_DECL_OVERRIDE;

    // QWidget interface
protected:
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

protected:
    virtual void changeProperties(const StdHandle& changingProperties);

protected:
    bool _isDirty;
    qint32 _contextIndex;
    QWidget* _view;
    ByteArrayPropertyPtr _savedGeometry;
    QMetaObject::Connection _connection;
};

#endif // PROPERTIESDIALOGBASE_H
