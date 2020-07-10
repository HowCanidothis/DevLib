#ifndef NOTIFYMANAGER_H
#define NOTIFYMANAGER_H

#include <QObject>
#include <QQueue>

class NotifyData;

class NotifyManager : public QObject
{
    Q_OBJECT

public:
    enum MessageType
    {
        Info,
        Warning,
        Error
    };

public:
    explicit NotifyManager( QObject* parent = 0);
    ~NotifyManager();

    static void Notify(MessageType messageType, const QString& body);
    static void Notify(QtMsgType qtMessageType, const QString& body);
    static NotifyManager& Instance();

    void SetMargins(qint32 bottom, qint32 right, qint32 space);
    void SetWidth(qint32 width);
    void SetDisplayTime(int ms);

private Q_SLOTS:
    void rearrange();
    void showNext();

private:
    QQueue<NotifyData*> m_dataQueue;
    QList<class NotifyWidget*> m_notifyList;
    qint32 m_bottom;
    qint32 m_right;
    qint32 m_space;
    qint32 m_width;
    qint32 m_displayTime;
    qint32 m_reservedHeight;
    qint32 m_freeHeight;
    qint32 m_maxHeight;
};

#endif // NOTIFYMANAGER_H
