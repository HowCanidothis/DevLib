#ifndef TABLEVIEWWIDGETATTACHMENT_H
#define TABLEVIEWWIDGETATTACHMENT_H

#include <QTableView>
#include <QComboBox>

#include <PropertiesModule/internal.hpp>

class TableViewColumnsWidgetAttachment : public QObject
{
    using Super = QObject;
    using CreateDelegate = std::function<QWidget* ()>;
public:
    TableViewColumnsWidgetAttachment(QTableView* targetTableView);
    ~TableViewColumnsWidgetAttachment();

    LocalPropertyBool IsVisible;
    DispatcherConnectionsSafe Connections;
    Dispatcher OnAttachmentsAdjusted;

    void Initialize(const CreateDelegate& createDelegate);

    qint32 GetAttachmentsCount() const { return m_attachmentWidgets.size(); }

    template<class T>
    T* GetAttachmentAt(qint32 index) const
    {
        auto foundIt = m_attachmentWidgets.find(index);
        if(foundIt != m_attachmentWidgets.end()) {
            return reinterpret_cast<T*>(foundIt.value());
        }
        return nullptr;
    }

    template<class T>
    qint32 IndexOf(T* widget) const
    {
        for(auto it(m_attachmentWidgets.begin()), e(m_attachmentWidgets.end()); it != e; it++) {
            if(it.value() == widget) {
                return it.key();
            }
        }
        return -1;
    }

    template<class T>
    void ForeachAttachment(const std::function<void (qint32, T*)>& onEveryAttachment)
    {
        for(auto it(m_attachmentWidgets.begin()), e(m_attachmentWidgets.end()); it != e; it++) {
            onEveryAttachment(it.key(), reinterpret_cast<T*>(it.value()));
        }
    }

private slots:
    void adjustAttachments(qint32 oldCount, qint32 newCount);
    void adjustGeometry();

private:
    QHash<qint32, QWidget*> m_attachmentWidgets;
    QTableView* m_targetTableView;
    CreateDelegate m_createDelegate;
    bool m_owner;
    QtLambdaConnections m_lconnections;
    DelayedCallObject m_adjustGeometry;
};

class WidgetsMatchingAttachment
{
public:
    WidgetsMatchingAttachment(QTableView* table, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns);

    LocalPropertyString DateFormat;
    LocalPropertyString DecimalSeparator;
    LocalPropertyBool IsVisible;
    LocalPropertyBool IsEnabled;
    Dispatcher Match;
    DelayedCallDispatcher Transite;
    Dispatcher OnTransited;
    CommonDispatcher<qint32, qint32> OnMatchingChanged;

    LocalPropertyErrorsContainer Errors;

    static const Name IncorrectDoubleConversionErrorName;
    static const Name IncorrectIntConversionErrorName;

private:
    bool hasHeader() const;
    void matchComboboxes();
    void match();

private:
    ScopedPointer<TableViewColumnsWidgetAttachment> m_attachment;
    QStringList m_requestedColumns;
    QTableView* m_tableView;
    ScopedPointer<FTSDictionary> m_dictionary;
    ScopedPointer<FTSObject> m_matchObject;
    QAbstractItemModel* m_targetModel;
};

#endif // TABLEVIEWWIDGETATTACHMENT_H
