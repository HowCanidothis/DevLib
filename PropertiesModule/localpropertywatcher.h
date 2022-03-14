#ifndef LOCALPROPERTYWATCHER_H
#define LOCALPROPERTYWATCHER_H

#include "localproperty.h"

class LocalPropertyWatcher
{
public:    
    template<class T>
    static DispatcherConnection Watch(const QString& name, LocalProperty<T>& property)
    {
        return property.OnChanged.Connect(nullptr, [name, &property]{
            qDebug() << name << "Changed" << property.Native();
        });
    }

    static DispatcherConnection Watch(const QString& name, Dispatcher& dispatcher)
    {
        return dispatcher.Connect(nullptr, [name]{
            qDebug() << name << "Changed";
        });
    }
};

#endif // LOCALPROPERTYWATCHER_H
