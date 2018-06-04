#include "gtwindow.h"
#include "ui_gtwindow.h"
#include "ComputeGraphModule/inputframestream.h"
#include <QComboBox>

#include <QKeyEvent>

#include "GraphicsToolsModule/gtcamera.h"
#include "computenodestreemodel.h"
#include "ComputeGraphModule/computegraphcore.h"
#include "ComputeGraphModule/computenodedepthfakesensor.h"
#include "ComputeGraphModule/computenodedelay.h"
#include "ComputeGraphModule/computenodevolcanorecognition.h"
#include "ComputeGraphModule/computenodeblur.h"
#include "ComputeGraphModule/computenodedepthsensor.h"

#include "PropertiesModule/propertieswindow.h"

#include <QOpenGLDebugLogger>

#include "SharedModule/external/qtqssreader.h"

GtWindow::GtWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GtWindow)
    , input_file(new InputFrameStream(1))
    , calibration_widget(new PropertiesWindow(this, Qt::Window))
    , main_qss("Common/QSS import file", GT_STYLES_PATH "main.qss")
    , full_screen("Output/fullscreen", false)
{
    ui->setupUi(this);

    full_screen.OnChange() = [this]{
        if(full_screen) {
            this->showFullScreenInternal(ui->glout1);
        }
        else {
            this->showNormalInternal(ui->glout1);
        }
    };

    QOpenGLDebugLogger* gllogger = new QOpenGLDebugLogger(this);
    connect(gllogger, &QOpenGLDebugLogger::messageLogged, [=](const QOpenGLDebugMessage& message){
        qWarning() << message.type() << message.message();
    });

    ui->glout1->setName("Output/left");
    ui->glout2->setName("Output/right");

    ui->glout1->setLogger(gllogger);
    ui->glout1->setShadowMapTechnique(true);
    ui->glout2->setShadowMapTechnique(true);

    ui->cmbStream->addItems(input_file->GetAvailableInputs());

    views.Push(ui->glout1);
    views.Push(ui->glout2);

    ComputeGraphCore* compute_graph = ComputeGraphCore::Instance();
    compute_graph->Initialize(input_file);

    ui->glout1->getOutputNode()->SetInput(compute_graph->GetComputeNodeBlurPost());
    ui->glout2->getOutputNode()->SetInput(compute_graph->GetComputeNodeDelay());

    ui->glout1->setVolcans(compute_graph->GetComputeNodeVolcanoRecognition());

    ui->glout1->setFpsBoard(ui->lbl_fps1);
    ui->glout1->setCpsBoard(ui->lbl_cps);
    ui->glout1->setLftBoard(ui->lbl_lft1);

    ui->glout2->setFpsBoard(ui->lbl_fps2);
    ui->glout2->setLftBoard(ui->lbl_lft2);

    ui->glout1->installEventFilter(this);
    ui->glout2->installEventFilter(this);

    GtComputeNodesTreeModel* model2 = new GtComputeNodesTreeModel(this);
    ui->tree_nodes->setModel(model2);

    connect(model2,
            SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            ui->tree_nodes,
            SLOT(expandAll()));
    model2->setRootNode(compute_graph->GetRootNode(), compute_graph);

    for(GtWidget3D* w : views) {
        w->setContextMenuPolicy(Qt::ActionsContextMenu);
    }

    for(GtComputeNodeBase* node : compute_graph->GetNodes()) {
        addChannel(node->GetName(), node);
    }

    QtQSSReader::InstallAndObserve(main_qss.ptr());
}

GtWindow::~GtWindow()
{
    ComputeGraphCore* compute_graph = ComputeGraphCore::Instance();
    compute_graph->Quit();
    delete ui;
}

void GtWindow::setCommonCamera(GtCamera* cam)
{
    ui->glout1->setCamera(cam);
    ui->glout2->setCamera(cam);

    cam->installObserver("camera");

    calibration_widget->LoadDefault();
}

void GtWindow::on_cmbStream_currentIndexChanged(const QString &arg1)
{
    input_file->SetFileName(arg1);
}

void GtWindow::onChannelTriggered()
{
    if(GtWidget3D* w = qobject_cast<GtWidget3D*>(focusWidget())) {
        QAction* action_channel = static_cast<QAction*>(sender());
        GtComputeNodeBase* compute_node = (GtComputeNodeBase*)action_channel->data().toLongLong();
        GtComputeNodeBase* out_node = w->getOutputNode();
        ComputeGraphCore::Instance()->Asynch([compute_node,out_node](){ out_node->SetInput(compute_node); });
        ComputeGraphCore::Instance()->ProcessEvents();

        GtComputeNodesTreeModel* model = (GtComputeNodesTreeModel*)ui->tree_nodes->model();
        model->update();
    }
}

void GtWindow::addChannel(const QString& name, GtComputeNodeBase* compute_node)
{
    QAction* action_channel = new QAction(name, this);
    for(GtWidget3D* view : views)
        view->addAction(action_channel);
    connect(action_channel, SIGNAL(triggered(bool)), this, SLOT(onChannelTriggered()));
    action_channel->setData((size_t)compute_node);
}

void GtWindow::setVisibleExcept(QWidget* parent, QWidget* widget_except, bool visible)
{
    QObjectList objects = parent->children();
    for(QObject* object : objects) {
        if(QWidget* w = qobject_cast<QWidget*>(object)) {
            if(w != widget_except) {
                w->setVisible(visible);
            }
        }
    }
}

void GtWindow::showFullScreenInternal(QWidget* view)
{
    showFullScreen();
    setVisibleExcept(ui->centralwidget, ui->widget_views, false);
    setVisibleExcept(ui->widget_views, view, false);
    view->setContextMenuPolicy(Qt::NoContextMenu);
    ui->statusbar->hide();
    ui->menubar->hide();
    calibration_widget->activateWindow();
}

void GtWindow::showNormalInternal(QWidget* view)
{
    showNormal();
    setVisibleExcept(ui->centralwidget, ui->widget_views, true);
    setVisibleExcept(ui->widget_views, view, true);
    view->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->statusbar->show();
    ui->menubar->show();
}

bool GtWindow::eventFilter(QObject* object, QEvent* event)
{
    switch(event->type()) {
    case QEvent::KeyRelease: {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if(key_event->key() == Qt::Key_Return) {
            input_file->SetPause(!input_file->IsPaused());
            return true;
        }
        QWidget* view = static_cast<QWidget*>(object);
        if(key_event->key() == Qt::Key_F) {
            showFullScreenInternal(view);
            return true;
        }
        if(key_event->key() == Qt::Key_N || key_event->key() == Qt::Key_Escape) {
            showNormalInternal(view);
            return true;
        }
        if(key_event->key() == Qt::Key_G) {
            calibration_widget->activateWindow();
            return true;
        }
        break;
    }
    case QEvent::MouseButtonDblClick: {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        QWidget* view = static_cast<QWidget*>(object);
        if(mouse_event->button() == Qt::LeftButton) {
            if(isFullScreen()) {
                showNormalInternal(view);
            }
            else {
                showFullScreenInternal(view);
            }
            return true;
        }
        break;
    }
    default: break;
    }
    return false;
}

void GtWindow::on_actionCalibration_triggered()
{
    calibration_widget->show();
}
