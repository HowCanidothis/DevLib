#ifndef WIDGETSSPINBOXLAYOUT_H
#define WIDGETSSPINBOXLAYOUT_H

#include <QWidget>

namespace Ui {
class WidgetsSpinBoxLayout;
}

class QLabel;
class QSpinBox;
class WidgetsSpinBoxLayout : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    explicit WidgetsSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsSpinBoxLayout();

    QLabel* label();
    QSpinBox* spinBox();

    QString title() const;

public slots:
    void setTitle(const QString& title);

private:
    Ui::WidgetsSpinBoxLayout *ui;
};

#endif // WIDGETSSPINBOXLAYOUT_H
