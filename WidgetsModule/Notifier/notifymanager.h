﻿#ifndef NOTIFYMANAGER_H
#define NOTIFYMANAGER_H

#include <QObject>
#include <QQueue>

#include <PropertiesModule/internal.hpp>

#include "notifydeclarations.h"

class NotifyData;

class NotifyManager : public QObject
{
    Q_OBJECT

public:
    enum MessageType
    {
        Info = 0x1,
        Warning = 0x2,
        Error = 0x4
    };

public:
    explicit NotifyManager( QObject* parent = 0);
    ~NotifyManager();

    void SetNotifyWidgetIcon(MessageType type, const IconsSvgIcon& icon);

    void Notify(MessageType messageType, const QString& body);
    void Notify(QtMsgType qtMessageType, const QString& body);
    static NotifyManager& GetInstance();

    LocalPropertyInt BottomMargin;
    LocalPropertyInt RightMargin;
    LocalPropertyInt Spacing;
    LocalPropertyInt DisplayTime;
    LocalPropertyInt Width;
    LocalPropertyInt Height;
    LocalPropertyInt ReservedHeight;
    LocalPropertyBool IsNotifactionsEnabled;
    QWidget* DefaultWindow;

    CommonDispatcher<const QString&, bool&> OnLinkActivated;
    CommonDispatcher<const NotifyDataPtr&> OnDataRecieved;

private Q_SLOTS:
    void rearrange();
    void showNext();
    QRect desktopRect() const;

private:
    QQueue<NotifyDataPtr> m_dataQueue;
    QList<class NotifyWidget*> m_notifyList;
    qint32 m_freeHeight;
    DispatchersCommutator m_onLayoutChanged;
    NotifyDataPtr m_exceedData;
    qint32 m_exceedCounter;
    IconsSvgIcon m_icons[Error + 1];
};

#endif // NOTIFYMANAGER_H
