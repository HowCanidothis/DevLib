#ifndef MAINPROGRESSBAR_H
#define MAINPROGRESSBAR_H

#include <QFrame>

#include <SharedModule/internal.hpp>

namespace Ui {
class MainProgressBar;
}

class MainProgressBar : public QFrame
{
    Q_OBJECT
    using Super = QFrame;

public:
    explicit MainProgressBar(const Name& processId, QWidget* parent = 0, Qt::WindowFlags windowFlags = Qt::Widget);
    ~MainProgressBar();

private:
    Ui::MainProgressBar *ui;
    DispatcherConnectionsSafe m_connections;
    qint32 m_counter;
};

#endif // MAINPROGRESSBAR_H
