#include "mainwindow.h"
#include <QApplication>
#include <QThread>
#include <ResourcesModule/internal.hpp>

class ThreadTest : public QThread
{
public:
    ThreadTest(QObject* par = 0)
        : QThread(par)
    {
        start();
    }

protected:
    void run() Q_DECL_OVERRIDE
    {
        forever {
            auto resource = ResourcesSystem::GetResource<int>("TestText");
            auto guard = resource->Data();
            int& value = guard.Change();
            value *= 2;
            qCWarning(LC_SYSTEM, "%d", guard.Get());
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    ResourcesSystem::RegisterResource("TestText", []{
        return new int(2);
    });

    ThreadTest t1, t2, t3, t4, t5, t6, t7;

    return a.exec();
}
