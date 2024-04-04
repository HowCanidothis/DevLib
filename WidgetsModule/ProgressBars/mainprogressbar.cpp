#include "mainprogressbar.h"
#include "ui_mainprogressbar.h"

#include <QResizeEvent>

#include <SharedModule/internal.hpp>

#include "WidgetsModule/Utils/widgethelpers.h"

MainProgressBar::MainProgressBar(const Name& processId, QWidget* parent, Qt::WindowFlags windowFlags)
    : Super(parent, windowFlags)
    , ui(new Ui::MainProgressBar)
    , m_counter(0)
{
    ui->setupUi(this);

    hide();

    ProcessFactory::Instance().OnDeterminate.Connect(CONNECTION_DEBUG_LOCATION, [this, processId](size_t, const DescProcessDeterminateValueState& value){
        if(!processId.IsNull() && value.Id != processId) {
            return;
        }
        setVisible(value.IsShouldStayVisible());
        ui->ProgressBar->SetMaximum(value.StepsCount);
        ui->ProgressBar->Value = value.CurrentStep;
        ui->ProgressBar->Text = value.Title;
    }).MakeSafe(m_connections);

    ProcessFactory::Instance().OnIndeterminate.Connect(CONNECTION_DEBUG_LOCATION, [this, processId](size_t, const DescProcessValueState& value){
        if(!processId.IsNull() && value.Id != processId) {
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
    }).MakeSafe(m_connections);

    WidgetWrapper(parentWidget()).AddEventFilter([this](QObject*, QEvent* e){
        if(e->type() == QEvent::Resize || e->type() == QEvent::Show) {
            auto* resizeEvent = reinterpret_cast<QResizeEvent*>(e);
            resize(resizeEvent->size());
            raise();
        }
        return false;
    });
}

MainProgressBar::~MainProgressBar()
{
    delete ui;
}
