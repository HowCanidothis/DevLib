#ifndef PROGRESSBARATTACHMENT_H
#define PROGRESSBARATTACHMENT_H

#include <QWidget>

class ProgressBarAttachment : public QObject
{
public:
    ProgressBarAttachment(QWidget* target);

    void EnableProgressBar();
    void EnableShadowProgressBar();

private:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget* m_target;
    class QFrame* m_progressBarPane;
    QFrame* m_shadowProgressBarPane;
};

#endif // PROGRESSBARATTACHMENT_H
