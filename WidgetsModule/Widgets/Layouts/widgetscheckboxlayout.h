#ifndef WIDGETSCHECKBOXLAYOUT_H
#define WIDGETSCHECKBOXLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsCheckBoxLayout;
}

class WidgetsCheckBoxLayout : public QFrame
{
    Q_OBJECT

public:
    explicit WidgetsCheckBoxLayout(QWidget *parent = nullptr);
    ~WidgetsCheckBoxLayout();

private:
    Ui::WidgetsCheckBoxLayout *ui;
};

#endif // WIDGETSCHECKBOXLAYOUT_H
