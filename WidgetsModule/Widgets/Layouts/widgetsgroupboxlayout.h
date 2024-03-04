#ifndef WIDGETSGROUPBOXLAYOUT_H
#define WIDGETSGROUPBOXLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsGroupBoxLayout;
}

class WidgetsGroupBoxLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    using Super = QFrame;

public:
    explicit WidgetsGroupBoxLayout(QWidget *parent = nullptr);
    ~WidgetsGroupBoxLayout();

    QWidget* widget() const;
    bool setWidget(QWidget* widget);

    QString title() const;

public slots:
    void setTitle(const QString& title);

private:
    Ui::WidgetsGroupBoxLayout *ui;
};

#endif // WIDGETSGROUPBOXLAYOUT_H
