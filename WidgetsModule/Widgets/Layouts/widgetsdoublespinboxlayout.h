#ifndef WIDGETSDOUBLESPINBOXLAYOUT_H
#define WIDGETSDOUBLESPINBOXLAYOUT_H

#include <QFrame>

#include <PropertiesModule/internal.hpp>

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
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)
    using Super = QFrame;

public:
    explicit WidgetsDoubleSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsDoubleSpinBoxLayout();

    class QCheckBox* checkBox() const { return m_checkbox == nullptr ? nullptr : m_checkbox->Checkbox; }
    QLabel* label() const;
    QLineEdit* lineEdit() const;
    QHBoxLayout* layout() const;
    WidgetsDoubleSpinBoxWithCustomDisplay* spinBox() const;

    bool readOnly() const;
    void setReadOnly(bool readOnly);

    bool checked() const;
    void setChecked(bool checked);
    bool checkable() const;
    void setCheckable(bool checkable);

    QString title() const;
    void setTitle(const QString& title);

private:
    void ensureCheckable();

private:
    struct CheckBoxComponent {
        QCheckBox* Checkbox;

        CheckBoxComponent();
        void Detach();
    };

    Ui::WidgetsDoubleSpinBoxLayout *ui;
    SharedPointer<CheckBoxComponent> m_checkbox;
};

#endif // WIDGETSDOUBLESPINBOXLAYOUT_H
