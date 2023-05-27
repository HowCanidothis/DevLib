#include "topnotifierframe.h"
#include "ui_topnotifierframe.h"

#include <QResizeEvent>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Attachments/widgetslocationattachment.h"
#include "WidgetsModule/Components/componentplacer.h"

TopNotifierFrame::TopNotifierFrame(QWidget* parent)
    : Super(parent)
    , ui(new Ui::TopNotifierFrame)
{
    ui->setupUi(this);

    m_placer = WidgetWrapper(this).LocateToParent(DescWidgetsLocationAttachmentParams(QuadTreeF::Location_MiddleTop).SetOffset({0,0}));

    hide();

    ui->Label->setTextFormat(Qt::RichText);
    ui->Label->setOpenExternalLinks(false);

    setWindowTitle(QString());

    WidgetLabelWrapper(ui->Label).OnLinkActivated().Connect(CONNECTION_DEBUG_LOCATION, [this](const Name& link){
        (*(FAction*)link.AsString().toLongLong())();
    });
}

TopNotifierFrame::~TopNotifierFrame()
{
    delete ui;
}

TranslatedString& TopNotifierFrame::WidgetText()
{
    return *WidgetLabelWrapper(ui->Label).WidgetText();
}

LocalProperty<QuadTreeF::BoundingRect_Location>& TopNotifierFrame::WidgetLocation()
{
    return m_placer->GetComponentPlacer()->Location;
}

LocalProperty<QPoint>& TopNotifierFrame::WidgetOffset()
{
    return m_placer->GetComponentPlacer()->Offset;
}

TopNotifierFrameErrorsComponent::TopNotifierFrameErrorsComponent(TopNotifierFrame* frame)
    : TopNotifierFrameErrorsComponent(new LocalPropertyErrorsContainer(), frame)
{
    m_internalErrors = true;
}

TopNotifierFrameErrorsComponent::TopNotifierFrameErrorsComponent(LocalPropertyErrorsContainer* errors, TopNotifierFrame* frame)
    : Super(frame)
    , m_updateText(1000)
    , m_errors(errors)
    , m_internalErrors(false)
{
    auto setText = m_updateText.Wrap(CONNECTION_DEBUG_LOCATION, [frame, errors]{
        frame->WidgetText().SetTranslationHandler([errors]{
            QString message;
            for(const auto& error : *errors) {
                message += error.Error->Native() + "\n";
            }
            if(!message.isEmpty()) {
                message.resize(message.size() - 1);
            }
            return message;
        });
        WidgetWrapper(frame).SetVisibleAnimated(!errors->IsEmpty());
    });
    errors->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, setText).MakeSafe(m_connections);
    errors->OnErrorsLabelsChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, setText).MakeSafe(m_connections);
}

TopNotifierFrameErrorsComponent::~TopNotifierFrameErrorsComponent()
{
    if(m_internalErrors) {
        delete m_errors;
    }
}
