#ifndef SHADOWPROGRESSBAR_H
#define SHADOWPROGRESSBAR_H

#include <QFrame>

#include <SharedModule/External/external.hpp>

namespace Ui {
class ShadowProgressBar;
}

class ShadowProgressBar : public QFrame
{
    Q_OBJECT

    class ProgressBarWithLabel : public QWidget
    {
    public:
        class QPushButton* CancelButton;
        class QLabel* Label;
        class QProgressBar* ProgressBar;

        ProgressBarWithLabel();

    private:
        QtLambdaConnections m_connections;
    };

public:
    explicit ShadowProgressBar(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~ShadowProgressBar();

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Ui::ShadowProgressBar *ui;
    ProgressBarWithLabel* m_progressBars[8];
    QSet<ProcessValue*> m_extraProcesses;
    QLabel* m_extraProcessesLabel;
};

#endif // SHADOWPROGRESSBAR_H
