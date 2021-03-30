#include "topnotifierframe.h"
#include "ui_topnotifierframe.h"

#include <QResizeEvent>

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

void TopNotifierFrame::SetText(const QString& text)
{
    ui->Label->setText(text);
    adjustSize();
}

void TopNotifierFrame::SetActionText(const QString& text)
{
    ui->BtnAction->setText(text);
    ui->BtnAction->setVisible(!text.isEmpty());
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
