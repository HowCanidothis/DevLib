#ifndef PROPERTIESWINDOW_H
#define PROPERTIESWINDOW_H

#include <QMainWindow>
#include <SharedModule/internal.hpp>

namespace Ui {
class PropertiesWindow;
}

class _Export PropertiesWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PropertiesWindow(QWidget *parent = 0, Qt::WindowFlags f=0);
    ~PropertiesWindow();

    void LoadDefault();

private slots:
    void on_actionSave_triggered();

    void on_actionSave_as_triggered();

    void on_actionLoad_triggered();

    void on_actionNew_triggered();

private:
    void load();
    void save();

    Ui::PropertiesWindow *m_ui;
    QString m_fileName;
};

#endif // PROPERTIESWINDOW_H
