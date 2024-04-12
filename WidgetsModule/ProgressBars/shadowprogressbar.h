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
    explicit ShadowProgressBar(QWidget *parent = 0, Qt::WindowFlags flags = Qt::Widget);
    ~ShadowProgressBar();

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Ui::ShadowProgressBar *ui;
    ProgressBarWithLabel* m_progressBars[8];
    QSet<size_t> m_extraProcesses;
    QLabel* m_extraProcessesLabel;
};

#endif // SHADOWPROGRESSBAR_H
