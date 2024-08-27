#ifndef WIDGETSSPINBOXLAYOUT_H
#define WIDGETSSPINBOXLAYOUT_H

#include <QFrame>

#include "widgetslayoutcomponent.h"

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
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(bool hasBox READ hasBox WRITE setHasBox)
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)
    using Super = QFrame;

public:
    explicit WidgetsSpinBoxLayout(QWidget *parent = nullptr);
    ~WidgetsSpinBoxLayout();

    class QCheckBox* checkBox() const { return m_checkbox == nullptr ? nullptr : m_checkbox->Widget; }
    QLabel* label() const;
    QLineEdit* box() const { return m_lineEdit == nullptr ? nullptr : m_lineEdit->Widget; }
    class QHBoxLayout* layout() const;
    WidgetsSpinBoxWithCustomDisplay* spinBox() const;

    QString title() const;
    void setTitle(const QString& title);

    bool readOnly() const;
    void setReadOnly(bool readOnly);

    bool hasBox() const;
    void setHasBox(bool hasBox);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);
    bool checked() const;
    void setChecked(bool checked);

    bool checkable() const;
    void setCheckable(bool checked);

private:
    void ensureCheckable();
    void ensureHasBox();

private:
    Ui::WidgetsSpinBoxLayout *ui;
    ScopedPointer<WidgetsLayoutComponent<QCheckBox>> m_checkbox;
    ScopedPointer<WidgetsLayoutComponent<QLineEdit>> m_lineEdit;
};

#endif // WIDGETSSPINBOXLAYOUT_H
