#ifndef GTWINDOW_H
#define GTWINDOW_H

#include <QMainWindow>
#include "SharedGuiModule/decl.h"
#include "SharedModule/stack.h"

#include "PropertiesModule/property.h"

namespace Ui {
class GtWindow;
}

class InputFrameStream;
class GtComputeNodeBase;
class PropertiesWindow;
class GtWidget3D;

class GtWindow : public QMainWindow
{
    typedef QMainWindow Super;
    Q_OBJECT
    PropertiesWindow* calibration_widget;
    InputFrameStream* input_file;
    Stack<GtComputeNodeBase*> channels;
    Stack<class GtWidget3D*> views;

    TextFileNameProperty main_qss;

    BoolProperty full_screen;
public:
    explicit GtWindow(QWidget *parent = 0);
    ~GtWindow();

    void setCommonCamera(class GtCamera* cam);

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;
private slots:
    void on_cmbStream_currentIndexChanged(const QString &arg1);

    void onChannelTriggered();
    void on_actionCalibration_triggered();

private:
    Ui::GtWindow *ui;

    template<class T> T* createNode() {
        T* res = new T();
        compute_nodes.push(res);
        addChannel(res->getName(), res);
        return res;
    }

    void addChannel(const QString& name, GtComputeNodeBase* compute_node);

    static void setVisibleExcept(QWidget* parent, QWidget* widget_except, bool visible);

    void showFullScreenInternal(QWidget* view);
    void showNormalInternal(QWidget* view);
};

#endif // GTWINDOW_H
