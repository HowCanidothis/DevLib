#ifndef WIDGETSDOUBLESPINBOXLAYOUT_H
#define WIDGETSDOUBLESPINBOXLAYOUT_H

#include <QFrame>

#include <PropertiesModule/internal.hpp>

#include "widgetslayoutcomponent.h"

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
    Q_PROPERTY(bool disable READ disable WRITE setDisable)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)
    Q_PROPERTY(bool hasBox READ hasBox WRITE setHasBox)
    Q_PROPERTY(bool hasRadioButton READ hasRadioButton WRITE setHasRadioButton)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    using Super = QFrame;

public:
    explicit WidgetsDoubleSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsDoubleSpinBoxLayout();

    class QCheckBox* checkBox() const { return m_checkbox == nullptr ? nullptr : m_checkbox->Widget; }
    QLabel* label() const;
    QLineEdit* lineEdit() const { return m_lineEdit == nullptr ? nullptr : m_lineEdit->Widget; }
    QHBoxLayout* layout() const;
    QHBoxLayout* headerLayout() const;
    WidgetsDoubleSpinBoxWithCustomDisplay* spinBox() const;
    class QRadioButton* radioButton() const { return m_radiobutton == nullptr ? nullptr : m_radiobutton->Widget; }

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

    bool readOnly() const;
    void setReadOnly(bool readOnly);

    bool checked() const;
    void setChecked(bool checked);
    bool checkable() const;
    void setCheckable(bool checkable);
    bool hasBox() const;
    void setHasBox(bool hasBox);
    bool hasRadioButton() const;
    void setHasRadioButton(bool hasRadioButton);

    QString title() const;
    void setTitle(const QString& title);

    bool disable() const;
    void setDisable(bool disable);

    LocalPropertyBool Disable;
private:
    void ensureCheckable();
    void ensureHasBox();
    void ensureRadioButton();

private:
    Ui::WidgetsDoubleSpinBoxLayout *ui;
    ScopedPointer<WidgetsLayoutComponent<QCheckBox>> m_checkbox;
    ScopedPointer<WidgetsLayoutComponent<QLineEdit>> m_lineEdit;
    ScopedPointer<WidgetsLayoutComponent<QRadioButton>> m_radiobutton;
};

#endif // WIDGETSDOUBLESPINBOXLAYOUT_H
