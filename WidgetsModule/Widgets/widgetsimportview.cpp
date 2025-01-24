#include "widgetsimportview.h"
#include "ui_widgetsimportview.h"

#include <QMenu>

#include <algorithm>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Attachments/tableviewwidgetattachment.h"
#include "WidgetsModule/Models/modelslistbase.h"

class VariantListModel : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
public:
    VariantListModel(QObject* parent = nullptr) : Super(parent) {}

    void FlipData();
    void SetData(const QList<QVector<QVariant>>& data);
    void SetData(const QList<QString>& data, const QString& separator);
    QList<QVector<QVariant>> GetData() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, qint32 role = Qt::EditRole) override
    {
        if(!index.isValid()) {
            return false;
        }
        if(role == Qt::EditRole) {
            m_data[index.row()][index.column()] = value;
            emit dataChanged(index, index);
            return true;
        }

        return false;
    }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    Qt::ItemFlags flags(const QModelIndex&) const override
    {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }

private:
    QList<QVector<QVariant>> m_data;
};

static QVector<QString> SEPARATORS = { " ", ";", "\t", "#", "|", "," };

WidgetsImportView::WidgetsImportView(QWidget *parent)
    : Super(parent)
    , ShowPreview(true)
    , GroupSeparator(GroupKeyboardSeparator::Tab)
    , ui(new Ui::WidgetsImportView)
{
	ui->setupUi(this);
    setWindowTitle(QString());
	
	auto* model = new VariantListModel(this);
    ui->SourceTable->setModel(model);

    MenuWrapper tableViewWrapper(ui->SourceTable);
    tableViewWrapper.AddDebugActions();
    tableViewWrapper.AddGlobalTableAction(GlobalActionCopyId);
    tableViewWrapper.AddGlobalTableAction(GlobalActionCopyWithHeadersId);
    tableViewWrapper.AddSeparator();
    tableViewWrapper.AddGlobalTableAction(GlobalActionPasteId);
    tableViewWrapper.AddSeparator();
    tableViewWrapper.AddGlobalTableAction(GlobalActionDeleteId);

    WidgetTableViewWrapper(ui->SourceTable).InitializeHorizontal(DescTableViewParams().SetUseStandardActions(false));
    auto handlers = WidgetTableViewWrapper(ui->SourceTable).CreateDefaultActionHandlers();
    handlers->FindHandler(GlobalActionDeleteId).SetAction([this]{
        auto* model = ui->SourceTable->model();
        auto indexs = WidgetTableViewWrapper(ui->SourceTable).SelectedRowsSorted();
        if(indexs.isEmpty()){
            return ;
        }
        int startSeries = indexs.last();
        int counter = 1;
        for(const auto& index : adapters::reverse(adapters::range(indexs.begin(), indexs.end()-1))){
            if(startSeries - counter != index){
                model->removeRows(startSeries - (counter - 1), counter);
                startSeries = index;
                counter = 1;
            } else {
                ++counter;
            }
        }
        model->removeRows(startSeries - (counter - 1), counter);
        if(indexs.first() == 0) {
            m_matchingAttachment->Match();
        } else {
            m_matchingAttachment->Transite();
        }
    });
    handlers->IsReadOnly = false;
    handlers->ShowAll();
	
    WidgetComboboxWrapper(ui->cbLocale).SetEnum<LocaleType>();
    WidgetComboboxWrapper(ui->cbGroup).SetEnum<GroupKeyboardSeparator>();
    WidgetComboboxWrapper(ui->cbDecimal).SetEnum<DecimalKeyboardSeparator>();
    m_connectors.AddConnector<LocalPropertiesComboBoxConnector>(&ImportLocale, ui->cbLocale);
    m_connectors.AddConnector<LocalPropertiesComboBoxConnector>(&GroupSeparator, ui->cbGroup);
    m_connectors.AddConnector<LocalPropertiesComboBoxConnector>(&DecimalSeparator, ui->cbDecimal);
	m_connectors.AddConnector<LocalPropertiesLineEditConnector>(&DateTimeFormat, ui->leDateTime);
    m_connectors.AddConnector<LocalPropertiesCheckBoxConnector>(&ShowPreview, ui->ShowPreview);
	
    ShowPreview.SetAndSubscribe([this]{
        ui->PreviewTable->setVisible(ShowPreview);
    });

    Locale.ConnectBoth(CONNECTION_DEBUG_LOCATION,ImportLocale, [](const QLocale& locale){
		switch(locale.language()) {
		case QLocale::Russian: return int(LocaleType::Russian);
		default: return int(LocaleType::English);
		}
	}, [](const int& type){
		switch (static_cast<LocaleType>(type)) {
		case LocaleType::Russian: return QLocale(QLocale::Russian);
		default: return QLocale(QLocale::English);
		}
	});
	
	Locale.SetAndSubscribe([this]{
        DecimalSeparator = TranslatorManager::GetNames<DecimalKeyboardSeparator>().indexOf(Locale.Native().decimalPoint());
        DateTimeFormat = Locale.Native().language() == QLocale::English ? "MM/dd/yyyy h:mm AP" : "dd.MM.yyyy H:mm";
	});
    DateTimeFormat.SetAndSubscribe([this]{ ui->lbDatePreview->lineEdit()->setText(QDateTime::currentDateTime().toString(DateTimeFormat)); });
	
    ui->SourceTable->setContextMenuPolicy(Qt::ActionsContextMenu);

    WidgetWrapper(this).FixUp();
}

WidgetsImportView::~WidgetsImportView()
{
	delete ui;
}

void WidgetsImportView::SetVisibilityMode(VisibilityFlags mode)
{
    ui->cbDecimal->setVisible(mode.TestFlag(VisibilityFlag_DecimalSeparator));

    ui->lbDatePreview->setVisible(mode.TestFlag(VisibilityFlag_Date));
    ui->leDateTime->setVisible(mode.TestFlag(VisibilityFlag_Date));

    ui->cbLocale->setVisible(mode.TestFlag(VisibilityFlag_Locale));

    ui->cbGroup->setVisible(mode.TestFlag(VisibilityFlag_GroupSeparator));
}

void WidgetsImportView::Initialize(const QList<QString>& data, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns)
{
	auto* model = GetModel();
    model->SetData(data, SEPARATORS[GroupSeparator]);
    GroupSeparator.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this, model, data]{ model->SetData(data, SEPARATORS[GroupSeparator]); }).MakeSafe(m_connections);
    initializeMatching(targetModel, targetImportColumns);
}

QTableView* WidgetsImportView::GetSourceTableView() const
{
    return ui->SourceTable;
}

QTableView* WidgetsImportView::GetPreviewTableView() const
{
    return ui->PreviewTable;
}

void WidgetsImportView::initializeMatching(QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns)
{
    m_matchingAttachment = new WidgetsMatchingAttachment(ui->SourceTable, targetModel, targetImportColumns);
    OnMatchingChanged.ConnectFromWithParameters(CONNECTION_DEBUG_LOCATION, m_matchingAttachment->OnMatchingChanged);
    m_matchingAttachment->IsEnabled = true;
    m_matchingAttachment->IsVisible = true;

    m_matchingAttachment->TransitionState.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        if(m_matchingAttachment->TransitionState) {
            IsInTransition = true;
        } else {
            IsInTransition = false;
        }
    });

    m_matchingAttachment->DateFormat.ConnectFrom(CONNECTION_DEBUG_LOCATION, DateTimeFormat);
    m_matchingAttachment->DecimalSeparator.ConnectFrom(CONNECTION_DEBUG_LOCATION, [](DecimalKeyboardSeparator key){
        return TranslatorManager::ToString(key);
    }, DecimalSeparator);
    ui->PreviewTable->setModel(targetModel);
}

void WidgetsImportView::SetPreviewModel(QAbstractItemModel* model)
{
    ui->PreviewTable->setModel(model);
}

void WidgetsImportView::Initialize(const QList<QVector<QVariant>>& data, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns)
{
    auto* model = GetModel();
    model->SetData(data);
    initializeMatching(targetModel, targetImportColumns);
    SetVisibilityMode(VisibilityFlag_Date | VisibilityFlag_DecimalSeparator | VisibilityFlag_Locale);
}

VariantListModel* WidgetsImportView::GetModel() const {
    return dynamic_cast<VariantListModel*>(ui->SourceTable->model());
}

void VariantListModel::SetData(const QList<QString>& data, const QString& separator){
    QList<QVector<QVariant>> swapData;
    qint32 maxCount = 0;
    for(const auto& stringRow : data){
        swapData.append(QVector<QVariant>());
        auto splited = stringRow.split(separator);
        for(const auto& split : splited) {
            swapData.last().append(split);
        }
        maxCount = std::max(maxCount, splited.size());
	}
    for(auto& data : swapData) {
        data.resize(maxCount);
    }

    beginResetModel();
	m_data.swap(swapData);
	endResetModel();
}

void VariantListModel::FlipData()
{
    QList<QVector<QVariant>> flipData;

    if(!m_data.isEmpty()) {
        auto count = m_data.constFirst().size();
        while(count--) {
            flipData.append(QVector<QVariant>(m_data.size()));
        }
        auto cRow = 0;
        for(const auto& r : adapters::reverse(m_data.cbegin(), m_data.cend())) {
            auto cCol = 0;
            for(const auto& v : r) {
                flipData[cCol][cRow] = v;
                ++cCol;
            }
            ++cRow;
        }
    }

    beginResetModel();
    m_data = flipData;
    endResetModel();
}

void VariantListModel::SetData(const QList<QVector<QVariant>>& data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}

QList<QVector<QVariant>> VariantListModel::GetData() const 
{
	QList<QVector<QVariant>> ret; ret.reserve(m_data.size());
	for(const auto& rowData : m_data){
		QVector<QVariant> variantRow; variantRow.reserve(rowData.size());
		for(const auto& str : rowData){
			variantRow.append(str);
		}
		ret.append(variantRow);
	}
	return ret;
}

int VariantListModel::rowCount(const QModelIndex& ) const {
	return m_data.count();
}

int VariantListModel::columnCount(const QModelIndex& ) const {
	return m_data.isEmpty() ? 0 : m_data.first().size();
}

QVariant VariantListModel::data(const QModelIndex& index, int role) const {
	if(!index.isValid()){
		return QVariant();
	}
	switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole: {
		const auto& row = m_data[index.row()];
        return row[index.column()];
		}
	default: return QVariant();
	}
	return QVariant();
}

bool VariantListModel::removeRows(int row, int count, const QModelIndex& parent) {
	beginRemoveRows(parent, row, row + (count - 1));
    qint32 index = 0; qint32 firstIndex = row; qint32 lastIndex = row + count - 1;
    auto end = std::remove_if(m_data.begin(), m_data.end(), [&index, firstIndex, lastIndex](const auto&) {
        if(index >= firstIndex && index <= lastIndex) {
            index++;
            return true;
        }
        index++;
        return false;
    });
    m_data.erase(end, m_data.end());
	endRemoveRows();
	return true;
}

void WidgetsImportView::on_BtnFlip_clicked()
{
    GetModel()->FlipData();
}

