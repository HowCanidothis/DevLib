#include "topnotifierframe.h"
#include "ui_topnotifierframe.h"

#include <QResizeEvent>

#include "WidgetsModule/Utils/widgethelpers.h"

TopNotifierFrame::TopNotifierFrame(QWidget* parent)
    : Super(parent)
    , ui(new Ui::TopNotifierFrame)
{
    ui->setupUi(this);
    ui->BtnAction->setVisible(false);

    move((parent->size().width() - width()) / 2, 0);

    parent->installEventFilter(this);
    hide();
}

TopNotifierFrame::~TopNotifierFrame()
{
    delete ui;
}

void TopNotifierFrame::SetText(const FTranslationHandler& text)
{
    WidgetLabelWrapper(ui->Label).WidgetText()->SetTranslationHandler(text);
    adjustSize();
}

void TopNotifierFrame::SetWidgetText(const FTranslationHandler& text)
{
    WidgetPushButtonWrapper(ui->BtnAction).WidgetText()->SetTranslationHandler(text);
    ui->BtnAction->setVisible(!text().isEmpty());
}

void TopNotifierFrame::SetAction(const FAction& action)
{
    connect(ui->BtnAction, &QPushButton::clicked, action);
}

bool TopNotifierFrame::eventFilter(QObject*, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Resize: {
        auto* resizeEvent = reinterpret_cast<QResizeEvent*>(event);
        move((resizeEvent->size().width() - width()) / 2, 0);
        break;
    }
    default: break;
    }

    return false;
}

TopNotifierFrameErrorsComponent::TopNotifierFrameErrorsComponent(LocalPropertyErrorsContainer* errors, TopNotifierFrame* frame)
    : Super(frame)
    , m_updateText(1000)
{
    auto setText = m_updateText.Wrap(CONNECTION_DEBUG_LOCATION, [frame, errors]{
        frame->SetText([errors]{
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
