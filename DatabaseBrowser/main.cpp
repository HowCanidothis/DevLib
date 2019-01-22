#include "databasebrowser.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DatabaseBrowser w;
    w.showMaximized();

    return a.exec();
}
