#ifndef WIDGETSHEADERLAYOUT_H
#define WIDGETSHEADERLAYOUT_H

#include <QFrame>

#include "widgetslayoutcomponent.h"

namespace Ui {
class WidgetsHeaderLayout;
}

class QLabel;
class QLineEdit;
class WidgetsHeaderLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString value READ value WRITE setValue)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(bool hasValue READ hasValue WRITE setHasValue)
    Q_PROPERTY(bool hasPrefix READ hasPrefix WRITE setHasPrefix)
    Q_PROPERTY(bool readOnlyPrefix READ readOnlyPrefix WRITE setReadOnlyPrefix)
    Q_PROPERTY(bool hasLockButton READ hasLockButton WRITE setHasLockButton)

public:
    explicit WidgetsHeaderLayout(QWidget *parent = nullptr);
    ~WidgetsHeaderLayout();

    QLabel* titleEdit() const;
    QLineEdit* prefixEdit() const;
    QLineEdit* valueEdit() const;
    QPushButton* lockButton() const;

    QString title() const;
    QString prefix() const;
    QString value() const;

    bool hasValue() const;
    bool hasPrefix() const;
    bool readOnlyPrefix() const;
    bool hasLockButton() const;

public slots:
    void setTitle(const QString& title);
    void setPrefix(const QString& prefix);
    void setValue(const QString& value);
    void setHasValue(bool value);
    void setHasPrefix(bool prefic);
    void setReadOnlyPrefix(bool readOnly);
    void setHasLockButton(bool enabled);

private:
    Ui::WidgetsHeaderLayout *ui;
    ScopedPointer<WidgetsLayoutComponent<QPushButton>> m_lockButton;
};

#endif // WIDGETSHEADERLAYOUT_H
