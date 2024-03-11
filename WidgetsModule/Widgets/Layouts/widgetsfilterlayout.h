#ifndef WIDGETSFILTERLAYOUT_H
#define WIDGETSFILTERLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsFilterLayout;
}

class QLabel;
class QCheckBox;
class WidgetsDoubleSpinBoxWithCustomDisplay;

class WidgetsFilterLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)

public:
    explicit WidgetsFilterLayout(QWidget *parent = nullptr);
    ~WidgetsFilterLayout();

    QLabel* label() const;
    QCheckBox* checkBox() const;
    WidgetsDoubleSpinBoxWithCustomDisplay* spinBox() const;

    QString title() const;
    bool checked() const;

public slots:
    void setTitle(const QString& title);
    void setChecked(const bool& checked);

private:
    Ui::WidgetsFilterLayout *ui;
};

#endif // WIDGETSFILTERLAYOUT_H
