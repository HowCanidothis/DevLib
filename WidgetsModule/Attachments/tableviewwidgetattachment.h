#ifndef TABLEVIEWWIDGETATTACHMENT_H
#define TABLEVIEWWIDGETATTACHMENT_H

#include <QTableView>
#include <QComboBox>

#include <SharedModule/internal.hpp>

class TableViewColumnsWidgetAttachment : public QObject
{
    using Super = QObject;
public:
    TableViewColumnsWidgetAttachment(QTableView* targetTableView);

    void SetWidget(qint32 columnIndex, QWidget* widget);
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

    qint32 IndexOf(QWidget* widget) const;

private:
    Stack<QWidget*> m_attachmentWidgets;
    QTableView* m_targetTableView;
};

class TableViewColumnsWidgetAttachmentDynamic : public QObject
{
    using Super = QObject;
    using CreateDelegate = std::function<QWidget* ()>;
public:
    TableViewColumnsWidgetAttachmentDynamic(QTableView* targetTableView);

    void Initialize(const CreateDelegate& createDelegate);
    void SetVisible(bool visible);

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
    void ForeachAttachment(const std::function<void (qint32, T*)>& onEveryAttachment)
    {
        for(auto it(m_attachmentWidgets.begin()), e(m_attachmentWidgets.end()); it != e; it++) {
            onEveryAttachment(it.key(), reinterpret_cast<T*>(it.value()));
        }
    }

private slots:
    void adjustAttachments(qint32 oldCount, qint32 newCount);

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    QHash<qint32, QWidget*> m_attachmentWidgets;
    QTableView* m_targetTableView;
    CreateDelegate m_createDelegate;
};

#endif // TABLEVIEWWIDGETATTACHMENT_H
