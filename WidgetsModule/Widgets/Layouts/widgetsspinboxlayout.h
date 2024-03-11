#ifndef WIDGETSSPINBOXLAYOUT_H
#define WIDGETSSPINBOXLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsSpinBoxLayout;
}

class QLabel;
class WidgetsSpinBoxWithCustomDisplay;
class WidgetsSpinBoxLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    using Super = QFrame;

public:
    explicit WidgetsSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsSpinBoxLayout();

    QLabel* label() const;
    WidgetsSpinBoxWithCustomDisplay* spinBox() const;

    QString title() const;
    void setTitle(const QString& title);

    bool readOnly() const;
    void setReadOnly(bool readOnly);

private:
    Ui::WidgetsSpinBoxLayout *ui;
};

#endif // WIDGETSSPINBOXLAYOUT_H
