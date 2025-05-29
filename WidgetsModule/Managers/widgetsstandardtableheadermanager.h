#ifndef WIDGETSSTANDARDTABLEATTACHMENT_H
#define WIDGETSSTANDARDTABLEATTACHMENT_H

#include "WidgetsModule/widgetsdeclarations.h"

class QHeaderView;

class WidgetsStandardTableHeaderManager : public SingletoneGlobal<WidgetsStandardTableHeaderManager>
{
    template<class T> friend class SingletoneGlobal;
    WidgetsStandardTableHeaderManager();
public:

    void Register(const DescTableViewParams& params, QHeaderView* headerView);

    template<class Buffer>
    void Serialize(Buffer& buffer)
    {
       buffer << buffer.Sect("States", m_states);
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
struct Serializer<QHash<Latin1Name, WidgetsStandardTableHeaderManager::StateObject>>
{
    typedef QHash<Latin1Name, WidgetsStandardTableHeaderManager::StateObject> target_type;
    using T = Latin1Name;
    using T2 = WidgetsStandardTableHeaderManager::StateObject;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        qint32 count = data.size();
        buffer.BeginKeyValueArray(buffer, count);
        for(auto it(data.begin()), e(data.end()); it != e; it++) {
            buffer.KeyValue(buffer, const_cast<T&>(it.key()), it.value().GetData()->CurrentState.EditSilent());
        }
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        qint32 count = data.size();
        buffer.BeginKeyValueArray(buffer, count);
        while(count--) {
            T key; QByteArray value;
            buffer.KeyValue(buffer, key, value);
            auto foundIt = data.find(key);
            if(foundIt == data.end()) {
                WidgetsStandardTableHeaderManager::StateObject object;
                object.Initialize(key);
                object.GetData()->CurrentState = value;
                data.insert(key, object);
            } else {
                foundIt.value().GetData()->CurrentState = value;
            }
        }
    }
};

#endif // WIDGETSSTANDARTTABLEATTACHMENT_H
