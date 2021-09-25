#include "widgetsimportview.h"
#include "ui_widgetsimportview.h"

#include <QMenu>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Attachments/tableviewwidgetattachment.h"

class VariantListModel : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
public:
    VariantListModel(QObject* parent = nullptr) : Super(parent) {}

    void SetData(const QList<QVector<QVariant>>& data);
    void SetData(const QList<QString>& data, const QString& separator);
    QList<QVector<QVariant>> GetData() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const  override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex& parent) override;

private:
    QList<QVector<QVariant>> m_data;
};

static QVector<QString> SEPARATORS = { " ", ";", "\t", "#", "|", "," };

WidgetsImportView::WidgetsImportView(QWidget *parent)
    : Super(parent)
    , ui(new Ui::WidgetsImportView)
{
	ui->setupUi(this);
	setWindowTitle("");
	
	auto* model = new VariantListModel(this);
    ui->SourceTable->setModel(model);
	
	ui->cbLocale->addItems(EnumHelper<LocaleType>::GetNames());
    ui->cbGroup->addItems(EnumHelper<GroupKeyboardSeparator>::GetNames());
    ui->cbDecimal->addItems(EnumHelper<DecimalKeyboardSeparator>::GetNames());
	m_connectors.AddConnector<LocalPropertiesComboBoxConnector>(&ImportLocale, ui->cbLocale);
	m_connectors.AddConnector<LocalPropertiesComboBoxConnector>(&GroupSeparator, ui->cbGroup);
	m_connectors.AddConnector<LocalPropertiesComboBoxConnector>(&DecimalSeparator, ui->cbDecimal);
	m_connectors.AddConnector<LocalPropertiesLineEditConnector>(&DateTimeFormat, ui->leDateTime);
	
	Locale.ConnectBoth(ImportLocale, [](const QLocale& locale){
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
        GroupSeparator = SEPARATORS.indexOf(Locale.Native().groupSeparator());
		DecimalSeparator = EnumHelper<DecimalKeyboardSeparator>::GetNames().indexOf(Locale.Native().decimalPoint());
		DateTimeFormat = Locale.Native().language() == QLocale::English ? "MM/dd/yyyy h:mm AP" : Locale.Native().dateTimeFormat(QLocale::ShortFormat);
	});
	DateTimeFormat.SetAndSubscribe([this]{ ui->lbDatePreview->setText(QDateTime::currentDateTime().toString(DateTimeFormat)); });
	
    ui->SourceTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->SourceTable, &QWidget::customContextMenuRequested, [this](const QPoint& pos){
        QMenu contextMenu(this);
		createAction(tr("Delete Row(s)"), [this]{
            auto* model = ui->SourceTable->model();
            auto indexs = WidgetContent::SelectedRowsSorted(ui->SourceTable);
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
		}, &contextMenu);
		
		///TODO: hide columns
//		createAction(tr("Delete Column(s)"), [this]{
//			auto* model = ui->SourceTable->model();
//			auto indexs = WidgetContent::SelectedColumnsSorted(ui->SourceTable);
			
//			int startSeries = indexs.last();
//			int counter = 1;
//			for(const auto& index : adapters::reverse(adapters::range(indexs.begin(), indexs.end()-1))){
//				if(startSeries - counter != index){
//					model->removeColumns(startSeries - (counter - 1), counter);
//					startSeries = index;
//					counter = 1;
//				} else {
//					++counter;
//				}
//			}
//			model->removeColumns(startSeries - (counter - 1), counter);
//		}, &contextMenu);
		
        contextMenu.exec(ui->SourceTable->viewport()->mapToGlobal(pos));
    });
}

WidgetsImportView::~WidgetsImportView()
{
	delete ui;
}

void WidgetsImportView::SetVisibilityMode(VisibilityFlags mode)
{
    ui->lbDecimal->setVisible(mode.TestFlag(VisibilityFlag_DecimalSeparator));
    ui->cbDecimal->setVisible(mode.TestFlag(VisibilityFlag_DecimalSeparator));

    ui->lbDatePreview->setVisible(mode.TestFlag(VisibilityFlag_Date));
    ui->lbDateTime->setVisible(mode.TestFlag(VisibilityFlag_Date));
    ui->lbDateTimeSample->setVisible(mode.TestFlag(VisibilityFlag_Date));
    ui->leDateTime->setVisible(mode.TestFlag(VisibilityFlag_Date));

    ui->lbLocale->setVisible(mode.TestFlag(VisibilityFlag_Locale));
    ui->cbLocale->setVisible(mode.TestFlag(VisibilityFlag_Locale));

    ui->lbGroupSeparator->setVisible(mode.TestFlag(VisibilityFlag_GroupSeparator));
    ui->cbGroup->setVisible(mode.TestFlag(VisibilityFlag_GroupSeparator));
}

void WidgetsImportView::Initialize(const QList<QString>& data, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns)
{
	auto* model = GetModel();
    model->SetData(data, SEPARATORS[GroupSeparator]);
    GroupSeparator.OnChange.Connect(this, [this, model, data]{ model->SetData(data, SEPARATORS[GroupSeparator]); }).MakeSafe(m_connections);
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
    OnMatchingChanged.ConnectFrom(m_matchingAttachment->OnMatchingChanged);
    OnTransited.ConnectFrom(m_matchingAttachment->OnTransited);
    m_matchingAttachment->DateFormat.ConnectFrom(DateTimeFormat);
    m_matchingAttachment->DecimalSeparator.ConnectFrom(DecimalSeparator, [](qint32 key){
        return EnumHelper<DecimalKeyboardSeparator>::GetNames().at(key);
    });
    ui->PreviewTable->setModel(targetModel);
    m_matchingAttachment->IsEnabled = true;
    m_matchingAttachment->IsVisible = true;
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
    for(const auto& stringRow : data){
        swapData.append(QVector<QVariant>());
        auto splited = stringRow.split(separator);
        for(const auto& split : splited) {
            swapData.last().append(split);
        }
	}
    beginResetModel();
	m_data.swap(swapData);
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
		return index.column() < row.size() ? row[index.column()] : QVariant();
		}
	default: return QVariant();
	}
	return QVariant();
}

bool VariantListModel::removeRows(int row, int count, const QModelIndex& parent) {
	beginRemoveRows(parent, row, row + (count - 1));
	while(count--){
        m_data.removeAt(row);
	}
	endRemoveRows();
	return true;
}

bool VariantListModel::removeColumns(int column, int count, const QModelIndex& parent) {
	beginRemoveColumns(parent, column, column + (count - 1));
	for(auto& rowData : m_data){
		auto counter = count;
		while(counter--){
			rowData.removeAt(column);
		}
	}
	endRemoveColumns();
	return true;
}
