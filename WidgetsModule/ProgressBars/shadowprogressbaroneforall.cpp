#include "shadowprogressbaroneforall.h"
#include "ui_shadowprogressbaroneforall.h"

ShadowProgressBarOneForAll::ShadowProgressBarOneForAll(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShadowProgressBarOneForAll)
    , m_stepsCount(-1)
    , m_step(-1)
{
    ui->setupUi(this);

    auto accumulateProcesses = [this](ProcessValue* value, bool visible, qint32 steps, qint32 stepsCount, const QString& title){
        ThreadsBase::DoMain([visible, value, this, steps, stepsCount, title]{
            if(visible) {
                if(!title.isEmpty()) {
                    m_processes.insert(value, title);
                }
                if(m_value.Native() == nullptr) {
                    m_step = steps;
                    m_stepsCount = stepsCount;
                    m_value = value;
                    m_label = m_processes[value];
                } else if(m_value.Native() == value) {
                    m_step = steps;
                    m_stepsCount = stepsCount;
                    m_label = m_processes[value];
                }
            } else {
                m_processes.remove(value);
                if(m_value.Native() == value) {
                    if(!m_processes.isEmpty()) {
                        m_step = 0;
                        m_stepsCount = 0;
                        m_value = m_processes.begin().key();
                        m_label = m_processes.begin().value();
                    } else {
                        m_value = nullptr;
                        m_label = "";
                    }
                }
            }
        });
    };

    ProcessFactory::Instance().SetShadowDeterminateCallback([accumulateProcesses](ProcessValue* value){
        bool visible = value->GetState().IsShouldStayVisible();
        qint32 steps = value->AsDeterminate()->GetCurrentStep();
        qint32 stepsCount = value->AsDeterminate()->GetStepsCount();
        accumulateProcesses(value, visible, steps, stepsCount, value->IsTitleChanged() ? value->GetTitle() : QString());
    });

    ProcessFactory::Instance().SetShadowIndeterminateCallback([accumulateProcesses](ProcessValue* value){
        bool visible = value->GetState().IsShouldStayVisible();
        qint32 steps = 0;
        qint32 stepsCount = 0;
        accumulateProcesses(value, visible, steps, stepsCount, value->IsTitleChanged() ? value->GetTitle() : QString());
    });

    m_value.Subscribe([this]{
        setVisible(m_value.Native() != nullptr);
    });
    m_step.Subscribe([this]{
        ui->ProgressBar->setValue(m_step);
    });

    m_stepsCount.Subscribe([this]{
        ui->ProgressBar->setMaximum(m_stepsCount);
    });

    m_label.Subscribe([this]{
        OnLabelChanged(m_label);
    });

    setVisible(false);
}

ShadowProgressBarOneForAll::~ShadowProgressBarOneForAll()
{
    delete ui;
}
