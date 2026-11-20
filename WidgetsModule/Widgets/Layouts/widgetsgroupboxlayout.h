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
//    Q_PROPERTY(bool removable READ removable WRITE setRemovable)
    Q_PROPERTY(qint32 gap READ gap WRITE setGap)
    using Super = QFrame;

public:
    explicit WidgetsGroupBoxLayout(QWidget *parent = nullptr);
    ~WidgetsGroupBoxLayout();

    QWidget* widget() const;
    bool setWidget(QWidget* widget);

    bool collapsable() const;
    void setCollapsable(bool collapsable);

//    bool removable() const;
//    void setRemovable(bool removable);

    qint32 gap() const;
    void setGap(qint32 gap);

    QString title() const;
    QPushButton* groupBar();

    LocalPropertyBool Opened;
    Dispatcher OnDelete;

public slots:
    void setTitle(const QString& title);

private:
    Ui::WidgetsGroupBoxLayout *ui;
    class QLabel* m_icon;
    class QPushButton* m_remove;
    bool m_collapsable;
//    bool m_removable;
};

#endif // WIDGETSGROUPBOXLAYOUT_H
