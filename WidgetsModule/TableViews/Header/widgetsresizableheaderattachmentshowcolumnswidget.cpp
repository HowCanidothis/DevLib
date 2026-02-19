#include "widgetsresizableheaderattachmentshowcolumnswidget.h"
#include "ui_widgetsresizableheaderattachmentshowcolumnswidget.h"

#include <QHeaderView>

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
#endif

#include "WidgetsModule/Models/modelstablebase.h"
#include "WidgetsModule/Models/modelsfiltermodelbase.h"
#include "WidgetsModule/Models/viewmodelsdefaultcomponents.h"
#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/widgetsdeclarations.h"

struct WidgetsResizableHeaderAttachmentShowColumnsData
{
    QString Text;
    qint32 LogicalIndex;
};

DECLARE_MODEL_BY_TYPE(WidgetsResizableHeaderAttachmentShowColumnsData);

class WidgetsResizableHeaderAttachmentShowColumnsViewModel : public TViewModelsTableBase<WidgetsResizableHeaderAttachmentShowColumnsDataModel>
{
    using Super = TViewModelsTableBase<WidgetsResizableHeaderAttachmentShowColumnsDataModel>;
public:
    WidgetsResizableHeaderAttachmentShowColumnsViewModel(QHeaderView* hv)
        : Super(hv)
        , m_hv(hv)
    {}

    bool setData(const QModelIndex& mi, const QVariant& v, qint32 role) override
    {
        if(!mi.isValid() || GetData() == nullptr) {
            return false;
        }
        if(role == Qt::CheckStateRole) {
            const auto& at = GetData()->At(mi.row());
            m_hv->setSectionHidden(at.LogicalIndex, v.toInt() != Qt::Checked);
            SectionChanged();
            m_hv->update();
            return true;
        }
        return false;
    }

    QVariant data(const QModelIndex& mi, qint32 role) const override
    {
        auto data = GetData();
        if(data == nullptr || !mi.isValid()) {
            return QVariant();
        }
        switch(role) {
        case Qt::CheckStateRole: {
            const auto& at = data->At(mi.row());
            return m_hv->isSectionHidden(at.LogicalIndex) ? Qt::Unchecked : Qt::Checked;
        }
        default: break;
        }
        return Super::data(mi, role);
    }

    Qt::ItemFlags flags(const QModelIndex& mi) const override
    {
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }

    Dispatcher SectionChanged;

private:
    QHeaderView* m_hv;
};

struct WidgetsResizableHeaderAttachmentFilterData
{
    WidgetsResizableHeaderAttachmentFilterData()
        : IsShown(Qt::PartiallyChecked)
    {
        OnChanged.ConnectFrom(CDL, Text, IsShown);
    }

    LocalPropertyString Text;
    LocalPropertyInt IsShown;

    bool HasFilter() const { return true; }

    Dispatcher OnChanged;
};

WidgetsResizableHeaderAttachmentShowColumnsWidget::WidgetsResizableHeaderAttachmentShowColumnsWidget(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsResizableHeaderAttachmentShowColumnsWidget)
    , m_viewModel(nullptr)
    , m_proxy(nullptr)
{
    ui->setupUi(this);
    WidgetAbstractButtonWrapper(ui->checkBox).SetOnClicked([this]{
        if(ui->checkBox->checkState() == Qt::PartiallyChecked) {
            ui->checkBox->setChecked(true);
        }
    });
}

WidgetsResizableHeaderAttachmentShowColumnsWidget::~WidgetsResizableHeaderAttachmentShowColumnsWidget()
{
    delete ui;
}

void WidgetsResizableHeaderAttachmentShowColumnsWidget::SetHeaderView(QHeaderView* hv, const DescTableViewParams& params)
{
    m_connectors.Clear();
    if(m_proxy != nullptr) {
        m_viewModel->deleteLater();
    }

    if(hv == nullptr) {
        m_proxy = nullptr;
        m_viewModel = nullptr;
        return;
    }

    auto model = ::make_shared<WidgetsResizableHeaderAttachmentShowColumnsDataModel>();
    for(qint32 i(0); i < hv->count(); ++i) {
        auto foundIt = params.ColumnsParams.constFind(i);
        if(foundIt != params.ColumnsParams.cend()) {
            if(!foundIt->CanBeHidden) {
                continue;
            }
        }
        model->EditSilent().append({ hv->model()->headerData(i, hv->orientation()).toString(), i });
    }

    auto* viewModel = new WidgetsResizableHeaderAttachmentShowColumnsViewModel(hv);
    viewModel->Builder().AddString(0, TR_NONE, &WidgetsResizableHeaderAttachmentShowColumnsData::Text).AddDefaultColors();
    viewModel->SetData(model);
    m_viewModel = viewModel;
    auto* proxy = new ViewModelsFilterModelBase(hv);
    proxy->setSourceModel(m_viewModel);
    m_proxy = proxy;

    auto* filterComponent = new ViewModelsTableFilterComponent<WidgetsResizableHeaderAttachmentFilterData>(proxy, [this](const WidgetsResizableHeaderAttachmentFilterData& f, qint32 index){
        auto checkState = m_viewModel->index(index, 0).data(Qt::CheckStateRole).toInt();
        if(f.IsShown != Qt::PartiallyChecked && f.IsShown != checkState) {
            return false;
        }
        if(!f.Text.Native().isEmpty()) {
            if(!m_viewModel->index(index, 0).data().toString().contains(f.Text, Qt::CaseInsensitive)) {
                return false;
            }
        }
        return true;
    });
    viewModel->SectionChanged.ConnectAndCall(CDL, [model, hv, filterComponent]{
        auto count = 0;
        auto hiddenCount = 0;
        for(const auto& c : *model) {
            if(hv->isSectionHidden(c.LogicalIndex)) {
                ++hiddenCount;
            }
            ++count;
        }
        if(hiddenCount == count) {
            filterComponent->FilterData.IsShown = Qt::Unchecked;
        } else if(hiddenCount == 0) {
            filterComponent->FilterData.IsShown = Qt::Checked;
        } else {
            filterComponent->FilterData.IsShown = Qt::PartiallyChecked;
        }
    });
    filterComponent->FilterData.IsShown.Connect(CDL, [this, hv, model](qint32 state) {
        if(state == Qt::PartiallyChecked) {
            return;
        }
        for(const auto& c : *model) {
            hv->setSectionHidden(c.LogicalIndex, !state);
        }
        ui->listView->update();
    });

    m_connectors.AddConnector<LocalPropertiesLineEditConnector>(&filterComponent->FilterData.Text, ui->lineEdit);
    m_connectors.AddConnector<LocalPropertiesCheckBoxConnector>(&filterComponent->FilterData.IsShown, ui->checkBox);
    ui->listView->setModel(proxy);
}
