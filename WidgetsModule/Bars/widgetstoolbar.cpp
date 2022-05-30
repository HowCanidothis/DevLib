#include "widgetstoolbar.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <WidgetsModule/internal.hpp>

WidgetsToolBar::WidgetsToolBar(QWidget* parent, Qt::Orientation orientation)
    : Super(parent)
    , m_locationAttachment(nullptr)
{
    switch(orientation){
    case Qt::Vertical: m_layout = new QVBoxLayout(this); break;
    case Qt::Horizontal: m_layout = new QHBoxLayout(this); break;
    }

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

void WidgetsToolBar::SetTarget(QWidget* target, QuadTreeF::BoundingRect_Location location)
{
    Q_ASSERT(m_locationAttachment == nullptr);
    m_locationAttachment = WidgetsLocationAttachment::Attach(DescWidgetsLocationAttachmentParams(this, location)
                                                             .SetRelativeParent(target)
                                                             .SetDelay(0));
}

ComponentPlacer* WidgetsToolBar::GetComponentPlacer() const
{
    return m_locationAttachment->GetComponentPlacer();
}

QPushButton* WidgetsToolBar::CreateDrawerButton(QWidget* drawer)
{
    auto* result = CreateButton("btnDrawer");
    auto* collapsed = &WidgetWrapper(drawer).WidgetCollapsing(true, 450);
    connect(result, &QPushButton::clicked, [collapsed](bool checked){
        *collapsed = checked;
    });
    collapsed->OnChanged.ConnectAndCall(this, [collapsed, result]{
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
