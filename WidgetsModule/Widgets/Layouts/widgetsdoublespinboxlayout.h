#ifndef WIDGETSDOUBLESPINBOXLAYOUT_H
#define WIDGETSDOUBLESPINBOXLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsDoubleSpinBoxLayout;
}

class QLabel;
class QLineEdit;
class QHBoxLayout;
class WidgetsDoubleSpinBoxWithCustomDisplay;
class WidgetsDoubleSpinBoxLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    using Super = QFrame;

public:
    explicit WidgetsDoubleSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsDoubleSpinBoxLayout();

    QLabel* label() const;
    QLineEdit* lineEdit() const;
    QHBoxLayout* layout() const;
    WidgetsDoubleSpinBoxWithCustomDisplay* spinBox() const;

    bool readOnly() const;
    void setReadOnly(bool readOnly);

    QString title() const;
    void setTitle(const QString& title);

private:
    Ui::WidgetsDoubleSpinBoxLayout *ui;
};

#endif // WIDGETSDOUBLESPINBOXLAYOUT_H
