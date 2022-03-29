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
    void updateState(const Latin1Name& stateName);
    virtual QByteArray saveState(QHeaderView* headerView);
    virtual void restoreState(const QByteArray& array, QHeaderView* headerView);

private:
    template<class T> friend struct Serializer;
    struct State
    {
        LocalProperty<QByteArray> CurrentState;
        QSet<QHeaderView*> Headers;
        DelayedCallObject UpdateState;

        State();

        void Update();
        void Connect(QHeaderView* header);
        void Initialize(const Latin1Name& stateName);

    private:
        QtLambdaConnections m_qtConnections;
        Latin1Name m_stateName;
    };

    class StateObject
    {
        SharedPointer<State> m_data;
    public:
        StateObject();
        void Initialize(const Latin1Name& stateName);

        const SharedPointer<State>& GetData() const { return m_data; }
    };

private:
    QHash<Latin1Name, StateObject> m_states;
};

template<>
struct Serializer<QHash<Latin1Name, WidgetsStandartTableHeaderManager::StateObject>>
{
    typedef QHash<Latin1Name, WidgetsStandartTableHeaderManager::StateObject> target_type;
    using T = Latin1Name;
    using T2 = WidgetsStandartTableHeaderManager::StateObject;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        qint32 count = data.size();
        buffer << buffer.Attr("Size", count);
        for(auto it(data.begin()), e(data.end()); it != e; it++) {
            buffer << buffer.Sect("key", const_cast<T&>(it.key()));
            buffer << buffer.Sect("value", it.value().GetData()->CurrentState.EditSilent());
        }
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        qint32 count = data.size();
        buffer << buffer.Attr("Size", count);
        while(count--) {
            T key; QByteArray value;
            buffer << buffer.Sect("key", key);
            buffer << buffer.Sect("value", value);
            auto foundIt = data.find(key);
            if(foundIt == data.end()) {
                WidgetsStandartTableHeaderManager::StateObject object;
                object.Initialize(key);
                object.GetData()->CurrentState = value;
                data.insert(key, object);
            } else {
                foundIt.value().GetData()->CurrentState = value;
            }
        }
    }
};

class WidgetsStandartTableAttachment
{
public:
    static QHeaderView* AttachHorizontal(class QTableView* tableView, const DescColumnsParams& params = DescColumnsParams());
    static QHeaderView* AttachVertical(class QTableView* tableView, const DescColumnsParams& params = DescColumnsParams());
};

#endif // WIDGETSSTANDARTTABLEATTACHMENT_H
