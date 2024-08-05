#ifndef MAINPROGRESSBAR_H
#define MAINPROGRESSBAR_H

#include <QFrame>

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class MainProgressBar;
}

class MainProgressBar : public QFrame
{
    Q_OBJECT
    using Super = QFrame;

public:
    explicit MainProgressBar(const QSet<Name>& processIds = QSet<Name>(), QWidget* parent = 0, Qt::WindowFlags windowFlags = 0);
    ~MainProgressBar();

    void SetProcessIds(const QSet<Name>& processIds);
private:
    Ui::MainProgressBar *ui;
    DispatcherConnectionsSafe m_connections;
    qint32 m_counter;
    QSet<Name> m_processIds;
};

#endif // MAINPROGRESSBAR_H
