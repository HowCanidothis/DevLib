#ifndef WIDGETSHEADERLAYOUT_H
#define WIDGETSHEADERLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsHeaderLayout;
}

class WidgetsHeaderLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool hasPrefix READ hasPrefix WRITE setHasPrefix)
    Q_PROPERTY(bool readOnlyPrefix READ readOnlyPrefix WRITE setReadOnlyPrefix)

public:
    explicit WidgetsHeaderLayout(QWidget *parent = nullptr);
    ~WidgetsHeaderLayout();

    QLineEdit* prefixEdit() const;
    QLineEdit* titleEdit() const;

    QString prefix() const;
    QString title() const;

    bool hasPrefix() const;
    bool readOnlyPrefix() const;

public slots:
    void setTitle(const QString& title);
    void setPrefix(const QString& prefix);
    void setHasPrefix(bool prefic);
    void setReadOnlyPrefix(bool readOnly);

private:
    Ui::WidgetsHeaderLayout *ui;
};

#endif // WIDGETSHEADERLAYOUT_H
