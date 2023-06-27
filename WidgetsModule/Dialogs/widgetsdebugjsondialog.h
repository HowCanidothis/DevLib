#ifndef WIDGETSDEBUGJSONDIALOG_H
#define WIDGETSDEBUGJSONDIALOG_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsDebugJsonDialog;
}

class WidgetsDebugJsonDialog : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetsDebugJsonDialog(QWidget *parent = nullptr);
    ~WidgetsDebugJsonDialog();

    void SetTableView(class QTableView* model);

private:
    Ui::WidgetsDebugJsonDialog *ui;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
    LocalPropertyString m_output;
    LocalPropertyString m_keyword;

    ArrayPointers<class QCheckBox> m_headerMap;
    QtLambdaConnections m_tvConnections;
    DelayedCallObject m_updateUi;
    DispatcherConnectionsSafe m_modelConnections;
};

#endif // WIDGETSDEBUGJSONDIALOG_H
