#include "tableviewwidgetattachment.h"

#include <QHeaderView>
#include <QScrollBar>

#include "WidgetsModule/Utils/widgethelpers.h"

const Name WidgetsMatchingAttachment::ErrorIncorrectDoubleConversion = "ErrorIncorrectDoubleConversion";
const Name WidgetsMatchingAttachment::ErrorIncorrectIntConversion = "ErrorIncorrectIntConversion";
const Name WidgetsMatchingAttachment::WarningAutoMatchDisabled = "WarningAutoMatchDisabled";

TableViewColumnsWidgetAttachment::TableViewColumnsWidgetAttachment(QTableView* targetTableView)
    : IsVisible(false)
    , m_targetTableView(targetTableView)
    , m_createDelegate(nullptr)
    , m_owner(true)
{
    IsVisible.Subscribe([this]{
        for(auto* attachment : m_attachmentWidgets) {
            attachment->setVisible(IsVisible);
        }
    });

    m_lconnections.connect(targetTableView, &QTableView::destroyed, [this]{
        m_owner = false;
    });
}

TableViewColumnsWidgetAttachment::~TableViewColumnsWidgetAttachment()
{
    if(m_owner) {
        for(const auto& widget : m_attachmentWidgets) {
            widget->deleteLater();
        }
    }
}

void TableViewColumnsWidgetAttachment::Initialize(const TableViewColumnsWidgetAttachment::CreateDelegate& createDelegate)
{
    Q_ASSERT(m_createDelegate == nullptr);
    m_createDelegate = createDelegate;
    adjustAttachments(0, m_targetTableView->horizontalHeader()->count());
    connect(m_targetTableView->horizontalHeader(), &QHeaderView::sectionCountChanged, this, &TableViewColumnsWidgetAttachment::adjustAttachments);
    connect(m_targetTableView->horizontalHeader(), &QHeaderView::sectionResized, this, &TableViewColumnsWidgetAttachment::adjustGeometry);
    connect(m_targetTableView->horizontalScrollBar(), &QScrollBar::valueChanged, this, &TableViewColumnsWidgetAttachment::adjustGeometry);
    adjustGeometry();
}

void TableViewColumnsWidgetAttachment::adjustAttachments(qint32 oldCount, qint32 newCount)
{
    if(oldCount < newCount) {
        qint32 counter = oldCount;
        while(counter != newCount) {
            auto* attachment = m_createDelegate();
            attachment->setParent(m_targetTableView->horizontalHeader());
            m_attachmentWidgets.insert(counter, attachment);
            attachment->setVisible(IsVisible);
            counter++;
        }
    } else if(oldCount > newCount) {
        qint32 counter = newCount;
        while(counter != oldCount) {
            auto foundIt = m_attachmentWidgets.find(counter);
            if(foundIt != m_attachmentWidgets.end()) {
                delete foundIt.value();
                m_attachmentWidgets.erase(foundIt);
            }
            counter++;
        }
    }
    OnAttachmentsAdjusted();
    adjustGeometry();
}

void TableViewColumnsWidgetAttachment::adjustGeometry()
{
    m_adjustGeometry.Call([this]{
        auto* tableView = m_targetTableView;
        if(tableView != nullptr) {
            qint32 columnsCount = tableView->model()->columnCount();
            for(qint32 i(0); i < columnsCount; ++i) {
                auto foundIt = m_attachmentWidgets.find(i);
                if(foundIt != m_attachmentWidgets.end()) {
                    QHeaderView* headerView = tableView->horizontalHeader();
                    auto x = headerView->sectionViewportPosition(i);
                    auto y = 0;
                    foundIt.value()->setGeometry(x + 2,y, headerView->sectionSize(i) - 4, headerView->height());
                    foundIt.value()->setCursor(QCursor(Qt::ArrowCursor));
                }
            }
        }
    });
}

WidgetsMatchingAttachment::WidgetsMatchingAttachment(QTableView* table, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns)
    : DecimalSeparator(".")
    , IsVisible(false)
    , IsEnabled(false)
    , TransitionState(false)
    , m_tableView(table)
    , m_dictionary(::make_scoped<FTSDictionary>())
    , m_matchObject(::make_scoped<FTSObject>(m_dictionary.get()))
    , m_targetModel(targetModel)
    , m_transite(1000)
{
    Match += { this, [this]{
        match();
    }};

    m_requestedColumns.append("-");
    if(targetImportColumns.isEmpty()) {
        for(qint32 i(0); i < targetModel->columnCount(); i++) {
            m_requestedColumns.append(targetModel->headerData(i, Qt::Horizontal).toString());
        }
    } else {
        for(qint32 i(0); i < targetModel->columnCount(); i++) {
            if(targetImportColumns.contains(i)) {
                m_requestedColumns.append(targetModel->headerData(i, Qt::Horizontal).toString());
            }
        }
    }

    IsEnabled.Subscribe([this]{
        if(IsEnabled) {
            m_attachment = new TableViewColumnsWidgetAttachment(m_tableView);
            m_attachment->Initialize([this]() -> QWidget* {
                                         auto* comboBox = new QComboBox();
                                         comboBox->setCursor(QCursor(Qt::ArrowCursor));
                                         comboBox->addItems(m_requestedColumns);
                                         comboBox->setCurrentIndex(0);
                                         QObject::connect(comboBox, QOverload<qint32>::of(&QComboBox::activated), [this, comboBox](qint32 index){
                                             m_attachment->ForeachAttachment<QComboBox>([this, index, comboBox](qint32 currentIndex, QComboBox* current) {
                                                 if(current != comboBox && current->currentIndex() == index) {
                                                     current->setCurrentIndex(0);
                                                     OnMatchingChanged(currentIndex, 0);
                                                 }
                                             });
                                             Transite();
                                             OnMatchingChanged(m_attachment->IndexOf(comboBox), index);
                                         });
                                         return comboBox;
                                     });

            m_attachment->IsVisible.ConnectFrom(IsVisible).MakeSafe(m_attachment->Connections);
            Match.ConnectFrom(m_attachment->OnAttachmentsAdjusted);
            match();
        } else {
            m_attachment = nullptr;
        }
    });

    m_transite.Connect(this, [this]{
        if(m_attachment == nullptr) {
            return;
        }

        QSet<qint32> doubleErrorsMetaData;
        QSet<qint32> intErrorsMetaData;

        int start = hasHeader() ? 1 : 0;
        auto* sourceModel = m_tableView->model();
        auto sourceCount = sourceModel->rowCount() - start;
        auto targetCount = m_targetModel->rowCount();
        targetCount -= m_targetModel->property("ExtraFieldsCount").toInt();
        if(sourceCount < targetCount){
            m_targetModel->removeRows(sourceCount, targetCount - sourceCount);
        } else if(targetCount < sourceCount){
            m_targetModel->insertRows(targetCount, sourceCount - targetCount);
        }

        QLocale doubleLocale(DecimalSeparator.Native() == ',' ? QLocale::Russian : QLocale::English);
        bool ok;
        auto targetColumnsCount = m_targetModel->columnCount();
        for(int row = 0; row < sourceCount; ++row){
            for(qint32 col(0); col < targetColumnsCount; col++) {
                auto targetIndex = m_targetModel->index(row, col);
                m_targetModel->setData(targetIndex, QVariant());
            }
        }

        m_attachment->ForeachAttachment<QComboBox>([&ok, sourceModel, this, &doubleLocale, start, sourceCount, &doubleErrorsMetaData, &intErrorsMetaData](qint32 sourceColumn, QComboBox* cb){
            auto targetColumn = cb->currentIndex() - 1;
            if(targetColumn < 0) {
                return;
            }
            for(int row = 0; row < sourceCount; ++row){
                auto sourceRow = row + start;
                auto value = sourceModel->data(sourceModel->index(sourceRow, sourceColumn));
                auto targetIndex = m_targetModel->index(row, targetColumn);
                if(value.type() == QVariant::String) {
                    auto type = m_targetModel->data(targetIndex, Qt::EditRole).type();
                    switch (type) {
                    case QVariant::DateTime: {
                        auto dt = doubleLocale.toDateTime(value.toString(), DateFormat);
                        m_targetModel->setData(targetIndex, dt);
                        break;
                    }
                    case QVariant::Double: {
                        auto dval = doubleLocale.toDouble(value.toString(), &ok);
                        if(!ok) {
                            doubleErrorsMetaData.insert(sourceRow);
                        }
                        m_targetModel->setData(targetIndex, dval);
                        break;
                    }
                    case QVariant::Int: {
                        auto ival = doubleLocale.toInt(value.toString(), &ok);
                        if(!ok){
                            ival = doubleLocale.toDouble(value.toString(), &ok);
                            if(!ok) {
                                intErrorsMetaData.insert(sourceRow);
                            }
                        }
                        m_targetModel->setData(targetIndex, ival);
                        break;
                    }
                    default: m_targetModel->setData(targetIndex, value); break;
                    }
                } else {
                    m_targetModel->setData(targetIndex, value);
                }
            }
        });

        Errors.ErrorsMetaData[ErrorIncorrectIntConversion] = QVariant::fromValue(intErrorsMetaData);
        Errors.ErrorsMetaData[ErrorIncorrectDoubleConversion] = QVariant::fromValue(doubleErrorsMetaData);

        if(!intErrorsMetaData.isEmpty()) {
            Errors.AddError(ErrorIncorrectIntConversion, QObject::tr("Unable to convert to integer value"), QtMsgType::QtWarningMsg);
        } else {
            Errors.RemoveError(ErrorIncorrectIntConversion);
        }

        if(!doubleErrorsMetaData.isEmpty()) {
            Errors.AddError(ErrorIncorrectDoubleConversion, QObject::tr("Unable to convert to double value"), QtMsgType::QtWarningMsg);
        } else {
            Errors.RemoveError(ErrorIncorrectDoubleConversion);
        }

        TransitionState.SetState(false);
    });

    auto transite = [this]{
        Transite();
    };
    auto match = [this]{
        Match();
    };

    DecimalSeparator.Subscribe(match);
    DateFormat.Subscribe(transite);

    m_lconnections.connect(m_tableView->model(), &QAbstractItemModel::dataChanged, transite);
}

void WidgetsMatchingAttachment::Transite()
{
    TransitionState.SetValue(true);
    m_transite();
}

void WidgetsMatchingAttachment::match()
{
    if(hasHeader()) {
        Errors.RemoveError(WarningAutoMatchDisabled);
        qint32 counter = 0;
        for(const auto& requestedColumn : m_requestedColumns) {
            m_matchObject->AddRow(requestedColumn, counter);
            counter++;
        }

        matchComboboxes();
    } else {
        Errors.AddError(WarningAutoMatchDisabled, QObject::tr("Can't recognize header row, automatic fields matching is disabled"), QtWarningMsg);
        m_attachment->ForeachAttachment<QComboBox>([this](qint32 index, QComboBox* comboBox){
            comboBox->setCurrentIndex(index + 1);
            OnMatchingChanged(index, index + 1);
        });
    }
    Transite();
}

bool WidgetsMatchingAttachment::hasHeader() const
{
    bool hasValidData = false;
    auto* model = m_tableView->model();
    bool ok;
    QLocale doubleLocale(DecimalSeparator.Native() == ',' ? QLocale::Russian : QLocale::English);
    for(int i=0; i<model->columnCount(); ++i){
        auto value = model->data(model->index(0, i));
        doubleLocale.toDouble(value.toString(), &ok);
        if(value.type() == QVariant::String && !ok){
            hasValidData = true;
        } else if(value.type() != QVariant::Invalid){
            return false;
        }
    }
    return hasValidData;
}

void WidgetsMatchingAttachment::matchComboboxes()
{
    Q_ASSERT(hasHeader());
    m_attachment->ForeachAttachment<QComboBox>([this](qint32 index, QComboBox* combobox){
        combobox->setCurrentIndex(0);
        OnMatchingChanged(index, 0);
    });
    QStringList list;
    auto* sourceModel = m_tableView->model();
    for(int i=0; i<sourceModel->columnCount(); ++i){
        list << sourceModel->data(sourceModel->index(0, i)).toString();
    }
    auto mapping = m_dictionary->Map(list);
    for(auto iter = mapping.begin(); iter != mapping.end(); ++iter){
        if(iter.value().Row.Object != nullptr){
            m_attachment->GetAttachmentAt<QComboBox>(iter.key())->setCurrentIndex(iter.value().Row.Id);
            OnMatchingChanged(iter.key(), iter.value().Row.Id);
        }
    }
}
