#ifndef RESOURCESSYSTEM_H
#define RESOURCESSYSTEM_H

#include <QString>
#include <functional>
#include <SharedModule/internal.hpp>

template<class T> class TResource;

class ResourcesSystem
{
    typedef QHash<Name, SharedPointer<class ResourceData>> ResourceCache;
    ResourceData* getResourceData(const Name& name) const;

public:
    ResourcesSystem() {}
    struct DescRegisterResourceParams
    {
        std::function<void*()> Initializer;
        std::function<void (void*)> Releaser;
        bool ThreadSafe;

        DescRegisterResourceParams(const std::function<void*()>& initializer, const std::function<void (void*)>& releaser, bool threadSafe)
            : Initializer(initializer)
            , Releaser(releaser)
            , ThreadSafe(threadSafe)
        {}

        template<class T>
        static DescRegisterResourceParams Create(const std::function<T*()>& initializer, bool threadSafe = false)
        {
            return DescRegisterResourceParams([initializer]{ return initializer(); }, [](void* data) { delete (T*)data; }, threadSafe);
        }
    };
    void RegisterResource(const Name& name, const DescRegisterResourceParams& params);

    template<class T>
    void RegisterResource(const Name& name, bool threadSafe = false)
    {
        RegisterResource(name, DescRegisterResourceParams::Create<T>([]() -> T* { return new T(); }, threadSafe));
    }

    template<class T>
    void RegisterResource(const Name& name, const std::function<T*()>& initializer, bool threadSafe = false)
    {
        RegisterResource(name, DescRegisterResourceParams::Create<T>(initializer, threadSafe));
    }

    template<class T>
    TResource<T> GetResource(const Name& name, bool silent = false) const {
        ResourceData* data = getResourceData(name);
        if(data == nullptr) {
            if(!silent) {
                qCWarning(LC_SYSTEM) << "trying to access undeclared resource" << name;
            }
            return TResource<T>();
        }
        return TResource<T>(data);
    }

private:
    ResourceCache m_resources;
};

#endif // RESOURCESSYSTEM_H
