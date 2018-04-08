#include "propertieswindow.h"
#include "ui_propertieswindow.h"
#include <QFileDialog>

PropertiesWindow::PropertiesWindow(QWidget *parent, Qt::WindowFlags f) :
    QMainWindow(parent, f),
    ui(new Ui::PropertiesWindow)
{
    ui->setupUi(this);
}

PropertiesWindow::~PropertiesWindow()
{
    delete ui;
}

void PropertiesWindow::loadDefault()
{
    file_name = "default.ini";
    load();
}

void PropertiesWindow::on_actionSave_triggered()
{
    if(file_name.isEmpty()) {
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
    file_name = res;
    save();
}

void PropertiesWindow::on_actionLoad_triggered()
{
    QString res = QFileDialog::getOpenFileName(this, tr("Select file to open"), QString(),"Settings (*.ini)");
    if(res.isEmpty()) {
        return;
    }
    file_name = res;
    load();
}

void PropertiesWindow::on_actionNew_triggered()
{
    file_name = "";
    setWindowTitle("");
}

void PropertiesWindow::load()
{
    setWindowTitle(file_name);
    ui->properties_view->load(file_name);
}

void PropertiesWindow::save()
{
    setWindowTitle(file_name);
    ui->properties_view->save(file_name);
}
