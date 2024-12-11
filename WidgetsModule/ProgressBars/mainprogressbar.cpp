#include "mainprogressbar.h"
#include "ui_mainprogressbar.h"

#include <QResizeEvent>

#include <SharedModule/internal.hpp>

#include "WidgetsModule/Utils/widgethelpers.h"

MainProgressBar::MainProgressBar(const QSet<Name>& processIds, QWidget* parent, Qt::WindowFlags windowFlags)
    : Super(parent, windowFlags)
    , ui(new Ui::MainProgressBar)
    , m_counter(0)
    , m_processIds(processIds)
{
    ui->setupUi(this);

    hide();

    ProcessFactory::Instance().OnDeterminate.Connect(CONNECTION_DEBUG_LOCATION, [this](size_t, const DescProcessDeterminateValueState& value){
        if(!m_processIds.contains(value.Id)) {
            return;
        }
        setVisible(value.IsShouldStayVisible());
        ui->ProgressBar->SetMaximum(value.StepsCount);
        ui->ProgressBar->Value = value.CurrentStep;
        ui->ProgressBar->Text = value.Title;
        if(value.IsFromMain) {
            ThreadsBase::ProcessUiOnly();
        }
    }).MakeSafe(m_connections);

    ProcessFactory::Instance().OnIndeterminate.Connect(CONNECTION_DEBUG_LOCATION, [this](size_t, const DescProcessValueState& value){
        if(!m_processIds.contains(value.Id)) {
            return;
        }
        bool visible = value.IsShouldStayVisible();
        if(!visible) {
            --m_counter;
        } else {
            ++m_counter;
        }
        setVisible(m_counter > 0);
        ui->ProgressBar->SetMaximum(0);
        ui->ProgressBar->Text = value.Title;
        if(value.IsFromMain) {
            ThreadsBase::ProcessUiOnly();
        }
    }).MakeSafe(m_connections);

    WidgetWrapper(parentWidget()).AddEventFilter([this](QObject*, QEvent* e){
        if(e->type() == QEvent::Resize || e->type() == QEvent::Show || e->type() == QEvent::ShowToParent) {
            auto* resizeEvent = reinterpret_cast<QResizeEvent*>(e);
            resize(resizeEvent->size());
            raise();
        }
        return false;
    });
    WidgetWrapper(this).AddEventFilter([this, parent](QObject*, QEvent* e) {
        if(e->type() == QEvent::ShowToParent || e->type() == QEvent::Show) {
            resize(parent->size());
            raise();
        }
        return false;
    });
}

MainProgressBar::~MainProgressBar()
{
    delete ui;
}

void MainProgressBar::SetProcessIds(const QSet<Name>& processIds)
{
    m_processIds = processIds;
}
