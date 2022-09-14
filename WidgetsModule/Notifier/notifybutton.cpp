#include "notifybutton.h"

#include <QLabel>
#include "WidgetsModule/Attachments/widgetslocationattachment.h"
#include "WidgetsModule/Components/componentplacer.h"
#include "WidgetsModule/Utils/widgethelpers.h"

NotifyButton::NotifyButton(QWidget* parent)
    : Super(parent)
    , NotificationsCount(0, 0, 99)
    , m_label(new QLabel(parent))
{
    NotificationsCountString.ConnectFrom(CONNECTION_DEBUG_LOCATION, NotificationsCount, [](qint32 count){
        if(count > 0) {
            if(count < 99) {
                return QString::number(count);
            } else {
                return QString::number(count) + "+";
            }
        }
        return QString("");
    });

    m_label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_label->setObjectName("NotificationCounter");

    m_connectors.AddConnector<LocalPropertiesLabelConnector>(&NotificationsCountString, m_label);

    auto* attachment = WidgetWrapper(m_label).LocateToParent(DescWidgetsLocationAttachmentParams(QuadTreeF::Location_TopRight)
                                          .SetOffset({m_offset.Native().width(), m_offset.Native().height()})
                                          .SetRelativeParent(this)
                                          .SetDelay(0));

    attachment->GetComponentPlacer()->Offset.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_offset, [](const QSize& size){
        return QPoint(size.width(), size.height());
    });

    NotificationsCountString.OnChanged += { this, [this]{
        ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[this]{
            if(NotificationsCount == 0) {
                m_label->setVisible(false);
            } else {
                m_label->setVisible(true);
                m_label->adjustSize();
            }
        });
    }};
    m_label->setVisible(false);
}

NotifyButton::~NotifyButton()
{
}
