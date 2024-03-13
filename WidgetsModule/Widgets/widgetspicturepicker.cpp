#include "widgetspicturepicker.h"

#include <QPushButton>

#include "WidgetsModule/Attachments/widgetslocationattachment.h"
#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsPicturePicker::WidgetsPicturePicker(QWidget* parent)
    : Super(parent)
{
    auto deleteButton = WidgetAbstractButtonWrapper(m_deleteButton = new QPushButton(this)).SetControl(ButtonRole::IconWithText).SetVisible(false).SetText(TR(tr("Remove")));
    auto addButton = WidgetAbstractButtonWrapper(m_addButton = new QPushButton(this)).SetControl(ButtonRole::IconWithText);
    deleteButton.LocateToParent(DescWidgetsLocationAttachmentParams());
    addButton->setObjectName("AddButton");
    deleteButton->setObjectName("DeleteButton");
    addButton.LocateToParent(DescWidgetsLocationAttachmentParams());
    addButton.WidgetVisibility().ConnectFrom(CDL, [](const QImage& image) {
        return image.isNull();
    }, Image);
}

void WidgetsPicturePicker::Initialize(LocalPropertiesWidgetConnectorsContainer& connectors, const FTranslationHandler& t, const QString& forceDefaultDir)
{
    WidgetAbstractButtonWrapper(m_addButton).SetText(t);
    connectors.AddConnector<LocalPropertiesLabelConnector>(&Image, this, LocalPropertiesLabelConnector::ImageConnectorParams().SetBrowseButton(m_addButton).SetClearButton(m_deleteButton).SetForceDefaultBrowseDir(forceDefaultDir));
}

void WidgetsPicturePicker::enterEvent(QEvent*)
{
    m_deleteButton->setVisible(!Image.Native().isNull());
}

void WidgetsPicturePicker::leaveEvent(QEvent*)
{
    m_deleteButton->setVisible(false);
}
