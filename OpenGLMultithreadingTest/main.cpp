#include "glwidget.h"
#include <QApplication>
#include <SharedModule/internal.hpp>
#include <SharedModule/External/external.hpp>
#include <ProgressModule/internal.hpp>

#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GLWidget w;
    w.show();
    w.StartRendering();

    QWidget* progressBarPanel = new QWidget(&w, Qt::Window);
    QVBoxLayout* layout = new QVBoxLayout(progressBarPanel);
    QLabel* progressBarLabel = new QLabel;
    QProgressBar* progressBar = new QProgressBar;
    layout->addWidget(progressBarLabel);
    layout->addWidget(progressBar);

    progressBarPanel->show();

    ProgressFactory::Instance().SetDeterminateCallback([progressBarLabel, progressBar, progressBarPanel](ProgressValue* value){
        if(value->GetDepth() != 0) { // Skip all subvalues
            return;
        }
        auto determinateValue = value->AsDeterminate();
        auto progressState = determinateValue->GetState();
        bool visible = progressState.IsShouldStayVisible();
        QtInlineEvent::Post([visible, progressState, progressBarLabel, progressBar, progressBarPanel]{
            progressBarPanel->setVisible(visible);
            progressBar->setMaximum(progressState.StepsCount);
            progressBar->setValue(progressState.CurrentStep);
            progressBarLabel->setText(progressState.Title);
        });
    });

    ProgressFactory::Instance().SetIndeterminateCallback([progressBarLabel, progressBar, progressBarPanel](ProgressValue* value){
        if(value->GetDepth() != 0) { // Skip all subvalues
            return;
        }
        auto progressState = value->GetState();
        bool visible = progressState.IsShouldStayVisible();
        QtInlineEvent::Post([visible, progressState, progressBarLabel, progressBar, progressBarPanel]{
            progressBarPanel->setVisible(visible);
            progressBar->setMaximum(0);
            progressBarLabel->setText(progressState.Title);
        });
    });

    return a.exec();
}
