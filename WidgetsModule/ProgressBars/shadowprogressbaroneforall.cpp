#include "shadowprogressbaroneforall.h"
#include "ui_shadowprogressbaroneforall.h"

static bool createdOne = false;

ShadowProgressBarOneForAll::ShadowProgressBarOneForAll(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShadowProgressBarOneForAll)
    , m_stepsCount(-1)
    , m_step(-1)
{
    ui->setupUi(this);

    Q_ASSERT(!createdOne);
    createdOne = true;

    auto accumulateProcesses = [this](size_t desc, bool visible, qint32 steps, qint32 stepsCount, const QString& title){
        if(visible) {
            if(!title.isEmpty()) {
                m_processes.insert(desc, title);
            }
            if(m_value.Native() == 0) {
                m_step = steps;
                m_stepsCount = stepsCount;
                m_value = desc;
                m_label = m_processes[desc];
            } else if(m_value.Native() == desc) {
                m_step = steps;
                m_stepsCount = stepsCount;
                m_label = m_processes[desc];
            }
        } else {
            m_processes.remove(desc);
            if(m_value.Native() == desc) {
                if(!m_processes.isEmpty()) {
                    m_step = 0;
                    m_stepsCount = 0;
                    m_value = m_processes.begin().key();
                    m_label = m_processes.begin().value();
                } else {
                    m_value = 0;
                    m_label = QString();
                }
            }
        }
    };

    ProcessFactory::Instance().OnDeterminate.Connect(CONNECTION_DEBUG_LOCATION, [accumulateProcesses](size_t desc, const DescProcessDeterminateValueState& value){
        bool visible = value.IsShouldStayVisible();
        qint32 steps = value.CurrentStep;
        qint32 stepsCount = value.StepsCount;
        accumulateProcesses(desc, visible, steps, stepsCount, value.IsTitleChanged ? value.Title : QString());
    });

    ProcessFactory::Instance().OnIndeterminate.Connect(CONNECTION_DEBUG_LOCATION, [accumulateProcesses](size_t desc, const DescProcessValueState& value){
        bool visible = value.IsShouldStayVisible();
        qint32 steps = 0;
        qint32 stepsCount = 0;
        accumulateProcesses(desc, visible, steps, stepsCount, value.IsTitleChanged ? value.Title : QString());
    });

    m_value.Subscribe([this]{
        setVisible(m_value.Native() != 0);
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
