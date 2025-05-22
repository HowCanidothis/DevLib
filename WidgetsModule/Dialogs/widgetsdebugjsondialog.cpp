#include "widgetsdebugjsondialog.h"
#include "ui_widgetsdebugjsondialog.h"

#include <QCheckBox>
#include <QTableView>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Managers/widgetsdialogsmanager.h"

WidgetsDebugJsonDialog::WidgetsDebugJsonDialog(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::WidgetsDebugJsonDialog)
    , m_keyword("%1")
{
    ui->setupUi(this);

    m_connectors.AddConnector<LocalPropertiesTextEditConnector>(&m_output, ui->teOutput);
    m_connectors.AddConnector<LocalPropertiesLineEditConnector>(&m_keyword, ui->leKeyword);

    setProperty(WidgetsDialogsManager::ResizeablePropertyName, true);

    m_keyword.SetValidator([](const QString& v) -> QString {
        if(!v.contains("%1")) {
            return "%1";
        }
        return v;
    });
}

WidgetsDebugJsonDialog::~WidgetsDebugJsonDialog()
{
    delete ui;
}

void WidgetsDebugJsonDialog::SetTableView(QTableView* tv)
{
    m_modelConnections.clear();
    m_tvConnections.Clear();
    auto* layout = ui->verticalLayout_2;
    while(auto* item = layout->takeAt(0)){
        delete item;
    }

    auto* model = tv->model();
    if(model == nullptr) {
        return;
    }

    bool vertical = model->property(WidgetProperties::InvertedModel).toBool();
    std::function<QCheckBox* (const QModelIndex& i)> hGetter;

    auto update = m_updateUi.Wrap(CONNECTION_DEBUG_LOCATION, [this, vertical, tv]{
        auto selected = vertical ? WidgetTableViewWrapper(tv).SelectedColumnsSorted() : WidgetTableViewWrapper(tv).SelectedRowsSorted();
        StringBuilder rString;
        for(auto row : selected) {
            StringBuilder current;
            for(qint32 j(0); j < m_headerMap.Size(); ++j) {
                auto* h = m_headerMap.At(j);
                QVariant data;
                if(h->isChecked() && !h->text().isEmpty()) {
                    if(vertical) {
                        data = tv->model()->index(j, row).data();
                    } else {
                        data = tv->model()->index(row, j).data();
                    }
                    bool toDouble;
                    data.toDouble(&toDouble);
                    QString pattern;
                    if(toDouble) {
                        pattern = "'%1': %2";
                    } else {
                        pattern = "'%1': '%2'";
                    }
                    current.Add(',', pattern.arg(h->text(), data.toString()));
                }
            }
            rString.Add('\n', m_keyword.Native().arg(current, QString::number(row)));
        }
        m_output = rString;
    });

    m_tvConnections.connect(tv->selectionModel(), &QItemSelectionModel::selectionChanged, update);
    m_keyword.OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, update).MakeSafe(m_modelConnections);

    qint32 count;
    Qt::Orientation orientation;
    if(vertical) {
        count = model->rowCount();
        orientation = Qt::Vertical;
    } else {
        count = model->columnCount();
        orientation = Qt::Horizontal;
    }
    m_headerMap.resizeAndAllocate(count);
    for(qint32 i(0); i < count; ++i) {
        auto h = model->headerData(i, orientation).toString(); h.remove('\n');
        QCheckBox* c = m_headerMap.at(i);
        WidgetCheckBoxWrapper(c).WidgetChecked().OnChanged.Connect(CONNECTION_DEBUG_LOCATION, update).MakeSafe(m_modelConnections);
        c->setText(h);
        layout->addWidget(c);
    }
    layout->addItem(new QSpacerItem(40,40, QSizePolicy::Minimum, QSizePolicy::Expanding));
}
