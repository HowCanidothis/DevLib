#include "propertieswindow.h"

#ifdef QT_GUI_LIB

#include "ui_propertieswindow.h"
#include <QFileDialog>

PropertiesWindow::PropertiesWindow(QWidget *parent, Qt::WindowFlags f) :
    QMainWindow(parent, f),
    m_ui(new Ui::PropertiesWindow)
{
    m_ui->setupUi(this);
}

PropertiesWindow::~PropertiesWindow()
{
    delete m_ui;
}

void PropertiesWindow::LoadDefault()
{
    m_fileName = "default.ini";
    load();
}

void PropertiesWindow::on_actionSave_triggered()
{
    if(m_fileName.isEmpty()) {
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
    m_fileName = res;
    save();
}

void PropertiesWindow::on_actionLoad_triggered()
{
    QString res = QFileDialog::getOpenFileName(this, tr("Select file to open"), QString(),"Settings (*.ini)");
    if(res.isEmpty()) {
        return;
    }
    m_fileName = res;
    load();
}

void PropertiesWindow::on_actionNew_triggered()
{
    m_fileName = "";
    setWindowTitle("");
}

void PropertiesWindow::load()
{
    setWindowTitle(m_fileName);
    m_ui->properties_view->Load(m_fileName);
}

void PropertiesWindow::save()
{
    setWindowTitle(m_fileName);
    m_ui->properties_view->Save(m_fileName);
}

#endif
