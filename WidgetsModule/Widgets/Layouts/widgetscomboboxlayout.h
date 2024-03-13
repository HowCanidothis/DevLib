#ifndef WIDGETSCOMBOBOXLAYOUT_H
#define WIDGETSCOMBOBOXLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsComboBoxLayout;
}

class QLabel;
class QComboBox;
class QHBoxLayout;
class WidgetsComboBoxLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool editable READ editable WRITE setEditable)
    using Super = QFrame;

public:
    explicit WidgetsComboBoxLayout(QWidget *parent = nullptr);
    ~WidgetsComboBoxLayout();

    QLabel* label() const;
    QComboBox* comboBox() const;
    QHBoxLayout* layout() const;

    QString title() const;
    bool editable() const;

public slots:
    void setTitle(const QString& title);
    void setEditable(const bool& editable);

private:
    Ui::WidgetsComboBoxLayout *ui;
};

#endif // WIDGETSCOMBOBOXLAYOUT_H
