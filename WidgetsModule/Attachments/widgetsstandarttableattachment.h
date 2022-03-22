#ifndef WIDGETSSTANDARTTABLEATTACHMENT_H
#define WIDGETSSTANDARTTABLEATTACHMENT_H

#include "WidgetsModule/widgetsdeclarations.h"

class QHeaderView;

class WidgetsStandartTableHeaderManager : public SingletoneGlobal<WidgetsStandartTableHeaderManager>
{
    template<class T> friend class SingletoneGlobal;
    WidgetsStandartTableHeaderManager();
public:

    void Register(const Latin1Name& stateName, QHeaderView* headerView);

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
       buffer.OpenSection("HeaderManager");
       buffer << buffer.Sect("States", m_states);
       buffer.CloseSection();
    }

protected:
    virtual QByteArray saveState(QHeaderView* headerView);
    virtual void restoreState(const QByteArray& array, QHeaderView* headerView);

private:
    struct State
    {
        SharedPointer<LocalProperty<QByteArray>> CurrentState;
        QSet<QHeaderView*> Headers;

        State()
            : CurrentState(::make_shared<LocalProperty<QByteArray>>())
        {}

        template<class Buffer>
        void Serialize(Buffer& buffer)
        {
           buffer << buffer.Sect("State", *CurrentState);
        }
    };

private:
    QHash<Latin1Name, State> m_states;
    DelayedCallObject m_updateStates;
};

class WidgetsStandartTableAttachment
{
public:
    static QHeaderView* AttachHorizontal(class QTableView* tableView, const DescColumnsParams& params = DescColumnsParams());
    static QHeaderView* AttachVertical(class QTableView* tableView, const DescColumnsParams& params = DescColumnsParams());
};

#endif // WIDGETSSTANDARTTABLEATTACHMENT_H
