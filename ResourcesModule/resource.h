#ifndef RESOURCE_H
#define RESOURCE_H

#include <Qt>
#include <functional>

class ResourceBase
{
    typedef std::function<void*()> LoadFunction;

    void** p_data;
    qint32* counter;
    LoadFunction loader;
public:
    ResourceBase(const LoadFunction& loader)
        : p_data((void**)malloc(sizeof(size_t)))
        , counter(new qint32)
        , loader(loader)
    {
        *p_data = nullptr;
        *counter = 0;
    }

    void setLoader(const LoadFunction& loader) {
        this->loader = loader;
    }

    void load() const{
        Q_ASSERT(*p_data == nullptr);
        *p_data = loader();
    }

    bool isNull() const {
         return *p_data == nullptr;
    }

    bool isValid() const {
        return !isNull();
    }

    template<typename T> friend class Resource;
};

template<typename T>
class Resource
{
    T** p_data;
    qint32* counter;
    Resource() {}
public:
    ~Resource() {
        if(--(*counter) == 0) {
            delete *p_data;
            *p_data = nullptr;
        }
    }

    Resource(const ResourceBase* other)
        : p_data(reinterpret_cast<T**>(other->p_data))
        , counter(other->counter)
    {
        (*counter)++;
        if(*p_data == nullptr) {
            other->load();
        }
    }

    bool isNull() const { return *p_data == nullptr; }
    const T* data() const { return *p_data; }
    T* data() { return *p_data; }
    T& get() { return **p_data; }
    const T& get() const { return **p_data; }
    void create(T* data) {
        Q_ASSERT(*p_data == nullptr);
        *p_data = data;
    }
private:
    Resource& operator=(const Resource& other) = delete;
};

#endif // RESOURCE_H
