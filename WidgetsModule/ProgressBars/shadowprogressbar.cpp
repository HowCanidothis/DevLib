#include "shadowprogressbar.h"
#include "ui_shadowprogressbar.h"

#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QPushButton>

#include <SharedModule/internal.hpp>

ShadowProgressBar::ShadowProgressBar(QWidget *parent, Qt::WindowFlags flags)
    : QFrame(parent, flags)
    , ui(new Ui::ShadowProgressBar)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_ShowWithoutActivating);

    parent->installEventFilter(this);

    for(auto*& progressBar : m_progressBars) {
        progressBar = new ProgressBarWithLabel();
        progressBar->setVisible(false);
        layout()->addWidget(progressBar);
    }

    layout()->addWidget(m_extraProcessesLabel = new QLabel);
    m_extraProcessesLabel->setVisible(false);

    ProcessFactory::Instance().SetShadowDeterminateCallback([this](ProcessValue* value){
        auto determinateValue = value->AsDeterminate();
        auto processState = determinateValue->GetState();
        bool visible = processState.IsShouldStayVisible();
        ThreadsBase::DoMain([visible, processState, value, this]{
            if(processState.Depth < 8) {
                auto* progressBar = m_progressBars[processState.Depth];
                progressBar->ProgressBar->setMaximum(processState.StepsCount);
                progressBar->ProgressBar->setValue(processState.CurrentStep);
                progressBar->CancelButton->setVisible(processState.IsCancelable);
                progressBar->CancelButton->setProperty("UsrValue", (processState.IsCancelable && visible) ? (size_t)value : 0);
                if(processState.IsTitleChanged) {
                    progressBar->Label->setText(QString::fromStdWString(processState.Title)); // TODO. copying
                }

                if(progressBar->isVisible() != visible) {
                    progressBar->setVisible(visible);
                    adjustSize();
                    move(parentWidget()->mapToGlobal(parentWidget()->rect().bottomRight()) - QPoint(width(), height()));
                }
            } else {
                if(visible) {
                    m_extraProcesses.insert(value);
                } else {
                    m_extraProcesses.remove(value);
                }

                auto visible = !m_extraProcesses.isEmpty();
                m_extraProcessesLabel->setText(QString(tr("Additional processes: %1").arg(m_extraProcesses.size())));
                if(visible != m_extraProcessesLabel->isVisible()) {
                    m_extraProcessesLabel->setVisible(visible);
                    adjustSize();
                    move(parentWidget()->mapToGlobal(parentWidget()->rect().bottomRight()) - QPoint(width(), height()));
                }
            }
        });
    });

    ProcessFactory::Instance().SetShadowIndeterminateCallback([this](ProcessValue* value){
        auto processState = value->GetState();
        bool visible = processState.IsShouldStayVisible();
        ThreadsBase::DoMain([visible, processState, value, this]{
            if(processState.Depth < 8) {
                auto* progressBar = m_progressBars[processState.Depth];
                progressBar->ProgressBar->setMaximum(0);
                progressBar->CancelButton->setVisible(processState.IsCancelable);
                progressBar->CancelButton->setProperty("UsrValue", (processState.IsCancelable && visible) ? (size_t)value : 0);
                if(processState.IsTitleChanged) {
                    progressBar->Label->setText(QString::fromStdWString(processState.Title)); // TODO. copying
                }
                if(progressBar->isVisible() != visible) {
                    progressBar->setVisible(visible);
                    adjustSize();
                    move(parentWidget()->mapToGlobal(parentWidget()->rect().bottomRight()) - QPoint(width(), height()));
                }
            }
        });
    });
}

ShadowProgressBar::~ShadowProgressBar()
{
    delete ui;
}

bool ShadowProgressBar::eventFilter(QObject* , QEvent* event)
{
    if(event->type() == QEvent::Move || event->type() == QEvent::LayoutRequest) {
        move(parentWidget()->mapToGlobal(parentWidget()->rect().bottomRight()) - QPoint(width(), height()));
    }
    return false;
}

ShadowProgressBar::ProgressBarWithLabel::ProgressBarWithLabel()
{
    setLayout(new QHBoxLayout());
    layout()->addWidget(CancelButton = new QPushButton(tr("Cancel")));
    layout()->addWidget(Label = new QLabel());
    layout()->addWidget(ProgressBar = new QProgressBar());

    m_connections.connect(CancelButton, &QPushButton::clicked, [this]{
        auto* value = reinterpret_cast<ProcessValue*>(CancelButton->property("UsrValue").toLongLong());
        if(value != nullptr) {
            value->Cancel();
        }
    });
}
