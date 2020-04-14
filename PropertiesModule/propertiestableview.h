#ifndef PROPERTIESTABLEVIEW_H
#define PROPERTIESTABLEVIEW_H

#include <QTableView>

#include "propertiesviewbase.h"
#include "propertiessystem.h"

class PropertiesTableView : public QTableView, protected PropertiesViewBase
{
    using Super = QTableView;
public:
    PropertiesTableView(QWidget* parent = 0, Qt::WindowFlags flags = 0);

    class PropertiesTableModel* GetModel() const;

private:
    PropertiesDelegate* propertiesDelegate() const override { return reinterpret_cast<PropertiesDelegate*>(itemDelegate()); }

private:
    class PropertiesTableModel* m_propertiesModel;
};

#endif // PROPERTIESTABLEVIEW_H
