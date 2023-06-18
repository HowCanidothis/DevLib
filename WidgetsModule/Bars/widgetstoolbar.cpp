#include "widgetstoolbar.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <WidgetsModule/internal.hpp>

WidgetsToolBar::WidgetsToolBar(QWidget* parent, Qt::Orientation orientation)
    : Super(parent)
{
    switch(orientation){
    case Qt::Vertical: m_layout = new QVBoxLayout(this); m_buttonsOrientation = Qt::Horizontal; break;
    case Qt::Horizontal: m_layout = new QHBoxLayout(this); m_buttonsOrientation = Qt::Vertical; break;
    }

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    raise();
}

void WidgetsToolBar::SetTarget(QWidget* target, QuadTreeF::BoundingRect_Location location)
{
    WidgetWrapper(this).LocateToParent(DescWidgetsLocationAttachmentParams(location)
                                       .SetRelativeParent(target)
                                       .SetDelay(0));
}

ComponentPlacer* WidgetsToolBar::GetComponentPlacer() const
{
    return WidgetWrapper(const_cast<WidgetsToolBar*>(this)).Location()->GetComponentPlacer();
}

QPushButton* WidgetsToolBar::CreateDrawerButton(QWidget* drawer, qint32 drawerSize)
{
    auto* result = CreateButton(m_buttonsOrientation == Qt::Horizontal ? "btnDrawer" : "btnDrawerVertical");
    auto* collapsed = &WidgetWrapper(drawer).WidgetCollapsing(m_buttonsOrientation, drawerSize);
    connect(result, &QPushButton::clicked, [collapsed](bool ){
        *collapsed = !collapsed->Native();
    });
    collapsed->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [collapsed, result]{
        result->setProperty("isOpen", collapsed->Native());
        StyleUtils::UpdateStyle(result);
    });
    return result;
}

QPushButton* WidgetsToolBar::CreateButton(const QString& objectName)
{
    auto* result = new QPushButton();
    result->setObjectName(objectName);
    result->setCheckable(true);
    result->setChecked(true);
    m_layout->addWidget(result);
    return result;
}
