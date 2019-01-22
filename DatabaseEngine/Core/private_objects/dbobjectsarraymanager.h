#ifndef DBOBJECTSARRAYMANAGER_H
#define DBOBJECTSARRAYMANAGER_H

#include <SharedModule/internal.hpp>

struct DbObjectsArrayManager
{
    template<class T>
    static void ReduceArray(ArrayPointers<T>& target){
        target.RemoveByPredicate([](T* t) {
            if(!t->IsValid()) {
                ScopedPointer<T> deleter(t); //ScopedPointer uses here as ~T() is private
                Q_UNUSED(deleter)
                return true;
            } return false;
        });
    }

    template<class T, typename Creator>
    static void ExpandArray(ArrayPointers<T>& target, Creator creator, qint32 last_id){
        qint32 dif = target.Size() - last_id;
        qint32 old_size = target.Size();
        target.Resize(last_id);
        auto it = target.begin();
        if(dif < 0) {
            for(T*& element : adapters::range(it + old_size, target.end())) {
                element = creator();
            }
        }
        else if(dif > 0) {
            for(T* element : adapters::range(target.end(), it + old_size)) {
                ScopedPointer<T> deleter(element); //ScopedPointer uses here as ~T() is private
                Q_UNUSED(deleter)
            }
        }
    }
};

#endif // DBOBJECTSARRAYMANAGER_H
