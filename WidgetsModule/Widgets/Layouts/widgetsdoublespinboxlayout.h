#ifndef WIDGETSDOUBLESPINBOXLAYOUT_H
#define WIDGETSDOUBLESPINBOXLAYOUT_H

#include <QWidget>

namespace Ui {
class WidgetsDoubleSpinBoxLayout;
}

class QLabel;
class QLineEdit;
class WidgetsDoubleSpinBoxWithCustomDisplay;
class WidgetsDoubleSpinBoxLayout : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool measurement READ measurement WRITE setMeasurement)

public:
    explicit WidgetsDoubleSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsDoubleSpinBoxLayout();

    QLabel* label();
    QLineEdit* lineEdit();
    WidgetsDoubleSpinBoxWithCustomDisplay* spinBox();

    QString title() const;
    bool measurement() const;

public slots:
    void setTitle(const QString& title);
    void setMeasurement(const bool& measurement);

private:
    Ui::WidgetsDoubleSpinBoxLayout *ui;
};

#endif // WIDGETSDOUBLESPINBOXLAYOUT_H
