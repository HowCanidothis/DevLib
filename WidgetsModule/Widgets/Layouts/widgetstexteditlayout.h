#ifndef WIDGETSTEXTEDITLAYOUT_H
#define WIDGETSTEXTEDITLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsTextEditLayout;
}

class QLabel;
class QTextEdit;
class WidgetsTextEditLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString placeHolder READ placeHolder WRITE setPlaceHolder)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    using Super = QFrame;
public:
    explicit WidgetsTextEditLayout(QWidget *parent = nullptr);
    ~WidgetsTextEditLayout();

    QLabel* label() const;
    QTextEdit *textEdit() const;

    QString title() const;
    void setTitle(const QString& title);

    QString placeHolder() const;
    void setPlaceHolder(const QString& placeHolder);

    bool readOnly() const;
    void setReadOnly(bool readOnly);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

private:
    Ui::WidgetsTextEditLayout *ui;
};

#endif // WIDGETSTEXTEDITLAYOUT_H
