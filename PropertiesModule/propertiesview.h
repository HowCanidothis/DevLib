#ifndef PROPERTIESVIEW_H
#define PROPERTIESVIEW_H

#include <QTreeView>
#include "property.h"

class PropertiesView : public QTreeView
{
    typedef QTreeView Super;
    class PropertiesModel* properties_model;
    QModelIndex index_under_cursor;

    QAction* action_open_with_text_editor;
    StringProperty text_editor;
    Q_OBJECT

public:
    PropertiesView(QWidget* parent=0, Qt::WindowFlags flags=0);

    void save(const QString& file_name);
    void load(const QString& file_name);
    // QWidget interface
protected:
    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    void validateActionsVisiblity();
private Q_SLOTS:
    void on_OpenWithTextEditor_triggered();
};

#endif // PROPERTIESVIEW_H
