#ifndef WIDGETSCOMBOBOXLAYOUT_H
#define WIDGETSCOMBOBOXLAYOUT_H

#include <QWidget>

namespace Ui {
class WidgetsComboBoxLayout;
}

class QLabel;
class QComboBox;
class WidgetsComboBoxLayout : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    explicit WidgetsComboBoxLayout(QWidget *parent = nullptr);
    ~WidgetsComboBoxLayout();

    QLabel* label();
    QComboBox* comboBox();

    QString title() const;

public slots:
    void setTitle(const QString& title);

private:
    Ui::WidgetsComboBoxLayout *ui;
};

#endif // WIDGETSCOMBOBOXLAYOUT_H
