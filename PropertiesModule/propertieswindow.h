#ifndef PROPERTIESWINDOW_H
#define PROPERTIESWINDOW_H

#include <QMainWindow>

namespace Ui {
class PropertiesWindow;
}

class PropertiesWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PropertiesWindow(QWidget *parent = 0, Qt::WindowFlags f=0);
    ~PropertiesWindow();

    void loadDefault();

private slots:
    void on_actionSave_triggered();

    void on_actionSave_as_triggered();

    void on_actionLoad_triggered();

    void on_actionNew_triggered();

private:
    void load();
    void save();

    Ui::PropertiesWindow *ui;
    QString file_name;
};

#endif // PROPERTIESWINDOW_H
