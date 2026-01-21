#ifndef WIDGETSRESIZABLEHEADERATTACHMENTSHOWCOLUMNSWIDGET_H
#define WIDGETSRESIZABLEHEADERATTACHMENTSHOWCOLUMNSWIDGET_H

#include <QFrame>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsResizableHeaderAttachmentShowColumnsWidget;
}

class WidgetsResizableHeaderAttachmentShowColumnsWidget : public QFrame
{
    Q_OBJECT

public:
    explicit WidgetsResizableHeaderAttachmentShowColumnsWidget(QWidget *parent = nullptr);
    ~WidgetsResizableHeaderAttachmentShowColumnsWidget();

    void SetHeaderView(QHeaderView* hv, const struct DescTableViewParams& params);

private:
    Ui::WidgetsResizableHeaderAttachmentShowColumnsWidget *ui;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
    QAbstractItemModel* m_viewModel;
    QAbstractItemModel* m_proxy;
};

#endif // WIDGETSRESIZABLEHEADERATTACHMENTSHOWCOLUMNSWIDGET_H
