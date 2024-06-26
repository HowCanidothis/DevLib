#ifndef WIDGETSLINEEDITLAYOUT_H
#define WIDGETSLINEEDITLAYOUT_H

#include <QFrame>
#include <QLineEdit>

namespace Ui {
class WidgetsLineEditLayout;
}

class QLabel;
class WidgetsLineEditLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString placeHolder READ placeHolder WRITE setPlaceHolder)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    Q_PROPERTY(QLineEdit::EchoMode echoMode READ echoMode WRITE setEchoMode)
    Q_PROPERTY(bool enableClearButton READ enableClearButton WRITE setEnableClearButton)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    using EchoMode = QLineEdit::EchoMode;
    using Super = QFrame;

public:
    explicit WidgetsLineEditLayout(QWidget *parent = nullptr);
    ~WidgetsLineEditLayout();  

    QLabel* label() const;
    QLineEdit* lineEdit() const;

    QString title() const;
    void setTitle(const QString& title);

    QString placeHolder() const;    
    void setPlaceHolder(const QString& placeHolder);

    bool enableClearButton() const;
    void setEnableClearButton(bool enabled);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

    bool readOnly() const;
    void setReadOnly(bool readOnly);

    EchoMode echoMode() const;
    void setEchoMode(EchoMode mode);
private:
    Ui::WidgetsLineEditLayout *ui;
};

#endif // WIDGETSLINEEDITLAYOUT_H
