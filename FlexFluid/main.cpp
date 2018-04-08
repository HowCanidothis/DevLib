#include <QApplication>
#include "openglwidget.h"

int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    OpenGLWidget w;
    w.show();

    return a.exec();
}
