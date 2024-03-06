#ifndef WIDGETSDOUBLESPINBOXLAYOUT_H
#define WIDGETSDOUBLESPINBOXLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsDoubleSpinBoxLayout;
}

class QLabel;
class QLineEdit;
class WidgetsDoubleSpinBoxWithCustomDisplay;
class WidgetsDoubleSpinBoxLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool measurement READ measurement WRITE setMeasurement)
    using Super = QFrame;

public:
    explicit WidgetsDoubleSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsDoubleSpinBoxLayout();

    QLabel* label() const;
    QLineEdit* lineEdit() const;
    WidgetsDoubleSpinBoxWithCustomDisplay* spinBox() const;
    QString title() const;
    bool measurement() const;

public slots:
    void setTitle(const QString& title);
    void setMeasurement(const bool& measurement);

private:
    Ui::WidgetsDoubleSpinBoxLayout *ui;
};

#endif // WIDGETSDOUBLESPINBOXLAYOUT_H
