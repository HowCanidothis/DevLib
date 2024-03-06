#ifndef WIDGETSGROUPBOXLAYOUT_H
#define WIDGETSGROUPBOXLAYOUT_H

#include <QFrame>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsGroupBoxLayout;
}

class WidgetsGroupBoxLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool collapsable READ collapsable WRITE setCollapsable)
    Q_PROPERTY(qint32 gap READ gap WRITE setGap)
    using Super = QFrame;

public:
    explicit WidgetsGroupBoxLayout(QWidget *parent = nullptr);
    ~WidgetsGroupBoxLayout();

    QWidget* widget() const;
    bool setWidget(QWidget* widget);

    bool collapsable() const;
    void setCollapsable(bool collapsable);

    qint32 gap() const;
    void setGap(qint32 gap);

    QString title() const;

    LocalPropertyBool Opened;

public slots:
    void setTitle(const QString& title);

private:
    Ui::WidgetsGroupBoxLayout *ui;
    class QLabel* m_icon;
    bool m_collapsable;
};

#endif // WIDGETSGROUPBOXLAYOUT_H
