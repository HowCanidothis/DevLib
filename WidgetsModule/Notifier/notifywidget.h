#ifndef NOTIFYWIDGET_H
#define NOTIFYWIDGET_H

#include <QFrame>

#include "notifydeclarations.h"

namespace Ui {
class NotifyWidget;
}

class NotifyWidget : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
public:
    explicit NotifyWidget(const NotifyDataPtr& data, QWidget* parent = 0);
    ~NotifyWidget();

    void ShowGriant(qint32 displayTime);

    CommonDispatcher<const QString&, bool&> OnLinkActivated;
    Dispatcher OnDisappeared;

private:
    Ui::NotifyWidget *ui;
    NotifyDataPtr m_data;

private:
    void hideGriant();
};

#endif // NOTIFYWIDGET_H
