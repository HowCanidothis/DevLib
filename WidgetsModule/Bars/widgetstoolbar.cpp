#include "widgetstoolbar.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <WidgetsModule/internal.hpp>

WidgetsToolBar::WidgetsToolBar(QWidget* parent, Qt::Orientation orientation)
    : Super(parent)
    , Expanded(nullptr)
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
    WidgetWrapper(this).ConnectVisibilityFrom(CDL, target);
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
    QString buttonObjectName;
    if(m_buttonsOrientation == Qt::Horizontal) {
        if(GetComponentPlacer()->Location.Native() & QuadTreeF::LeftLessZero) {
            buttonObjectName = "btnDrawer";
        } else {
            buttonObjectName = "btnDrawerM";
        }
    } else {
        if(GetComponentPlacer()->Location.Native() & QuadTreeF::TopLessZero) {
            buttonObjectName = "btnDrawerVertical";
        } else {
            buttonObjectName = "btnDrawerVerticalM";
        }
    }
    auto* result = CreateButton(buttonObjectName);
    Expanded = &WidgetWrapper(drawer).WidgetCollapsing(m_buttonsOrientation, drawerSize);
    connect(result, &QPushButton::clicked, [this](bool ){
        *Expanded = !Expanded->Native();
    });
    Expanded->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this, result]{
        result->setProperty("isOpen", Expanded->Native());
        WidgetWrapper(result).UpdateStyle();
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
