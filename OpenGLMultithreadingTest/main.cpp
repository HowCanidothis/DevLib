#include "glwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GLWidget w;
    w.show();
    w.StartRendering();

    return a.exec();
}
