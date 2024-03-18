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
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)
    using Super = QFrame;

public:
    explicit WidgetsSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsSpinBoxLayout();

    class QCheckBox* checkBox() const { return m_checkbox == nullptr ? nullptr : m_checkbox->Checkbox; }
    QLabel* label() const;
    WidgetsSpinBoxWithCustomDisplay* spinBox() const;

    QString title() const;
    void setTitle(const QString& title);

    bool readOnly() const;
    void setReadOnly(bool readOnly);

    bool checked() const;
    void setChecked(bool checked);
    bool checkable() const;
    void setCheckable(bool checkable);
private:
    void ensureCheckable();

private:
    struct CheckBoxComponent {
        QCheckBox* Checkbox;

        CheckBoxComponent();
        void Detach();
    };
    Ui::WidgetsSpinBoxLayout *ui;
    ScopedPointer<CheckBoxComponent> m_checkbox;
};

#endif // WIDGETSSPINBOXLAYOUT_H
