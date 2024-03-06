#ifndef WIDGETSCOMBOBOXLAYOUT_H
#define WIDGETSCOMBOBOXLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsComboBoxLayout;
}

class QLabel;
class QComboBox;
class WidgetsComboBoxLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    using Super = QFrame;

public:
    explicit WidgetsComboBoxLayout(QWidget *parent = nullptr);
    ~WidgetsComboBoxLayout();

    QLabel* label() const;
    QComboBox* comboBox() const;

    QString title() const;

public slots:
    void setTitle(const QString& title);

private:
    Ui::WidgetsComboBoxLayout *ui;
};

#endif // WIDGETSCOMBOBOXLAYOUT_H
