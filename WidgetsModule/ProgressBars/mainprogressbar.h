#ifndef MAINPROGRESSBAR_H
#define MAINPROGRESSBAR_H

#include <QFrame>

namespace Ui {
class MainProgressBar;
}

class MainProgressBar : public QFrame
{
    Q_OBJECT
    using Super = QFrame;

public:
    explicit MainProgressBar(QWidget *parent = 0, Qt::WindowFlags windowFlags = 0);
    ~MainProgressBar();

    // QObject interface
public:
    virtual bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;

private:
    class QRoundProgressBar* getProgressBarFromDepth(qint32 depth) const;

private:
    Ui::MainProgressBar *ui;
};

#endif // MAINPROGRESSBAR_H
