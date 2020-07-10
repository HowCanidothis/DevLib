#include "mainprogressbar.h"
#include "ui_mainprogressbar.h"

#include <SharedModule/internal.hpp>

MainProgressBar::MainProgressBar(QWidget *parent, Qt::WindowFlags windowFlags)
    : Super(parent, windowFlags)
    , ui(new Ui::MainProgressBar)
{
    ui->setupUi(this);
    ui->SubProgressBar->installEventFilter(this);

    ui->ProgressBar->hide();
    ui->SubProgressBar->hide();

    setWindowModality(Qt::WindowModal);

    ProcessFactory::Instance().SetDeterminateCallback([this](ProcessValue* value){
        auto determinateValue = value->AsDeterminate();
        auto processState = determinateValue->GetState();
        bool visible = processState.IsShouldStayVisible();
        QRoundProgressBar* progressBar = getProgressBarFromDepth(processState.Depth);
        if(progressBar == nullptr) {
            return;
        }
        ThreadsBase::DoMain([visible, processState, progressBar, this]{
            if(progressBar->isVisible() != visible) {
                if(processState.Depth == 0) {
                    setVisible(visible);
                }
                progressBar->setVisible(visible);
                auto topLeft = parentWidget()->mapToGlobal(parentWidget()->rect().topLeft());
                setGeometry(QRect(topLeft, parentWidget()->size()));
            }
            progressBar->setMaximum(processState.StepsCount);
            progressBar->setValue(processState.CurrentStep);
            if(processState.IsTitleChanged) {
                progressBar->setText(QString::fromStdWString(processState.Title));
            }
        });
    });

    ProcessFactory::Instance().SetIndeterminateCallback([this](ProcessValue* value){
        auto processState = value->GetState();
        bool visible = processState.IsShouldStayVisible();
        QRoundProgressBar* progressBar = getProgressBarFromDepth(processState.Depth);
        if(progressBar == nullptr) {
            return;
        }
        ThreadsBase::DoMain([visible, processState, progressBar, this]{
            if(progressBar->isVisible() != visible) {
                if(processState.Depth == 0) {
                    setVisible(visible);
                }
                progressBar->setVisible(visible);
                auto topLeft = parentWidget()->mapToGlobal(parentWidget()->rect().topLeft());
                setGeometry(QRect(topLeft, parentWidget()->size()));
            }
            progressBar->setMaximum(0);
            if(processState.IsTitleChanged) {
                progressBar->setText(QString::fromStdWString(processState.Title));
            }
        });
    });
}

MainProgressBar::~MainProgressBar()
{
    delete ui;
}

QRoundProgressBar* MainProgressBar::getProgressBarFromDepth(qint32 depth) const
{
    switch (depth) {
        case 0: return ui->ProgressBar;
        case 1: return ui->SubProgressBar;
        default: return nullptr;
    }
}

bool MainProgressBar::eventFilter(QObject*, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Resize: {
        auto subHalfWidth = ui->SubProgressBar->width() / 2;
        auto subHalfHeight = ui->SubProgressBar->height() / 2;
        auto halfWidth = ui->ProgressBar->width() / 2;
        auto halfHeight = ui->ProgressBar->height() / 2;
        ui->SubProgressBar->move(halfWidth - subHalfWidth, halfHeight - subHalfHeight);
        break;
    }
    default: break;
    }
    return false;
}
