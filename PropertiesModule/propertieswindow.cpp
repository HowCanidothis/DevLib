#include "propertieswindow.h"
#include "ui_propertieswindow.h"
#include <QFileDialog>

PropertiesWindow::PropertiesWindow(QWidget *parent, Qt::WindowFlags f) :
    QMainWindow(parent, f),
    _ui(new Ui::PropertiesWindow)
{
    _ui->setupUi(this);
}

PropertiesWindow::~PropertiesWindow()
{
    delete _ui;
}

void PropertiesWindow::LoadDefault()
{
    _fileName = "default.ini";
    load();
}

void PropertiesWindow::on_actionSave_triggered()
{
    if(_fileName.isEmpty()) {
        on_actionSave_as_triggered();
    }
    else {
        save();
    }
}

void PropertiesWindow::on_actionSave_as_triggered()
{
    QString res = QFileDialog::getSaveFileName(this, tr("Save file as..."), QString(),"Settings (*.ini)");
    if(res.isEmpty()) {
        return;
    }
    _fileName = res;
    save();
}

void PropertiesWindow::on_actionLoad_triggered()
{
    QString res = QFileDialog::getOpenFileName(this, tr("Select file to open"), QString(),"Settings (*.ini)");
    if(res.isEmpty()) {
        return;
    }
    _fileName = res;
    load();
}

void PropertiesWindow::on_actionNew_triggered()
{
    _fileName = "";
    setWindowTitle("");
}

void PropertiesWindow::load()
{
    setWindowTitle(_fileName);
    _ui->properties_view->Load(_fileName);
}

void PropertiesWindow::save()
{
    setWindowTitle(_fileName);
    _ui->properties_view->Save(_fileName);
}
