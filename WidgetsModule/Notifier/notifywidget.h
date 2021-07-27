#ifndef NOTIFY_H
#define NOTIFY_H

#include <QLabel>

#include <SharedModule/internal.hpp>

class QPushButton;

class NotifyData {
public:
    NotifyData(qint32 type, const QString& body, const QString& extendedBody, const QString& icon = "", const QString& url = "")
        : Icon(icon)
        , Body(body)
        , ExtendedBody(extendedBody)
        , Url(url)
        , Type(type)
    {}

    QString Icon;
    QString Body;
    QString ExtendedBody;
    QString Url;
    qint32 Type;
};

class NotifyWidget : public QLabel
{
    Q_OBJECT
public:
    explicit NotifyWidget(NotifyData* data, int m_displayTime, QWidget* parent = 0);

    void ShowGriant(qint32 maxHeight);

    CommonDispatcher<const QString&, bool&> LinkActivated;

Q_SIGNALS:
    void disappeared();

private:
    int m_displayTime;
    bool m_autoHide;

    ScopedPointer<NotifyData> m_data;

    QLabel* m_iconLabel;
    QLabel* m_bodyLabel;

    QPushButton* m_closeBtn;

    void hideGriant();

    void mousePressEvent(QMouseEvent* event);
};

#endif // NOTIFY_H
