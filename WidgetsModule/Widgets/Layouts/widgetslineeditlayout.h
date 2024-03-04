#ifndef WIDGETSLINEEDITLAYOUT_H
#define WIDGETSLINEEDITLAYOUT_H

#include <QWidget>

namespace Ui {
class WidgetsLineEditLayout;
}

class QLabel;
class QLineEdit;
class WidgetsLineEditLayout : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString placeHolder READ placeHolder WRITE setPlaceHolder)

public:
    explicit WidgetsLineEditLayout(QWidget *parent = nullptr);
    ~WidgetsLineEditLayout();

    QLabel* label();
    QLineEdit* lineEdit();

    QString title() const;
    QString placeHolder() const;

public slots:
    void setTitle(const QString& title);
    void setPlaceHolder(const QString& placeHolder);

private:
    Ui::WidgetsLineEditLayout *ui;
};

#endif // WIDGETSLINEEDITLAYOUT_H
