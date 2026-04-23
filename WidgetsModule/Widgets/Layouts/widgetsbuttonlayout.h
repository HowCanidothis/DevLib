#ifndef WIDGETSBUTTONLAYOUT_H
#define WIDGETSBUTTONLAYOUT_H

#include <QFrame>

#include "widgetslayoutcomponent.h"

namespace Ui {
class WidgetsButtonLayout;
}

class QLabel;
class QComboBox;
class QHBoxLayout;
class WidgetsButtonLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool hasButton READ hasButton WRITE setHasButton)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    using Super = QFrame;

public:
    explicit WidgetsButtonLayout(QWidget *parent = nullptr);
    ~WidgetsButtonLayout();

    QPushButton* button() const { return m_button == nullptr ? nullptr : m_button->Widget; }
    QLabel* label() const;
    QPushButton* pushButton() const;
    QHBoxLayout* valueLayout() const;
    QHBoxLayout* headerLayout() const;

    bool hasButton() const { return m_button != nullptr; }
    void setHasButton(bool has);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

    QString title() const;

public slots:
    void setTitle(const QString& title);

private:
    Ui::WidgetsButtonLayout *ui;
    ScopedPointer<WidgetsLayoutComponent<QPushButton>> m_button;
};

#endif // WidgetsButtonLayout_H
