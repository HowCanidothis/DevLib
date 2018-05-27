#include <QApplication>

#include "gtwindow.h"
#include "GraphicsToolsModule/gtcamera.h"
#include "ComputeGraphModule/computegraphcore.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    GtCamera cam;

    cam.setProjectionProperties(45.f,20.f,15000.f);

    cam.setSceneBox(BoundingBox(Point3F(-10000.f,-10000.f, 3.f), Point3F(10000.f,10000.f,10000.f)));
    cam.setIsometric(false);
    cam.setPosition(Point3F(320.f,220.f,1200.f), Vector3F(0.f,0.f,-1.f), Vector3F(0.f,1.f,0.f));

    qInfo("Initializing windows...");

    GtWindow w;
    w.setCommonCamera(&cam);
    w.show();

    ComputeGraphCore::Instance()->Start();

    qInfo("Initializing windows completed");

    return a.exec();
}
