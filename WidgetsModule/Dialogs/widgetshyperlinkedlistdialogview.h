#ifndef WIDGETSHYPERLINKEDLISTDIALOGVIEW_H
#define WIDGETSHYPERLINKEDLISTDIALOGVIEW_H

#include <QWidget>

namespace Ui {
class WidgetsHyperlinkedListDialogView;
}

class WidgetsHyperlinkedListDialogView : public QWidget
{
    Q_OBJECT
    using Super = QWidget;
public:
    explicit WidgetsHyperlinkedListDialogView(QWidget *parent = nullptr);
    ~WidgetsHyperlinkedListDialogView();

    void SetText(const QString& text, const QString& content);

private:
    Ui::WidgetsHyperlinkedListDialogView *ui;
};

#endif // WIDGETSHYPERLINKEDLISTDIALOGVIEW_H
