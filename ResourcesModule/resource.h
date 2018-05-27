#ifndef RESOURCE_H
#define RESOURCE_H

#include <Qt>
#include <functional>

class ResourceBase
{
    typedef std::function<void*()> LoadFunction;

    void** _data;
    qint32* _counter;
    LoadFunction _loader;
public:
    ResourceBase(const LoadFunction& loader);

    void load() const;
    bool isNull() const { return *_data == nullptr; }
    bool isValid() const { return !isNull(); }

    template<typename T> friend class Resource;
};

template<typename T>
class Resource
{
    T** _data;
    qint32* _counter;
    Resource() {}
public:
    ~Resource()
    {
        if(--(*_counter) == 0) {
            delete *_data;
            *_data = nullptr;
        }
    }

    Resource(const ResourceBase* other)
        : _data(reinterpret_cast<T**>(other->_data))
        , _counter(other->_counter)
    {
        (*_counter)++;
        if(*_data == nullptr) {
            other->load();
        }
    }

    bool IsNull() const { return *_data == nullptr; }
    const T* Data() const { return *_data; }
    T* Data() { return *_data; }
    T& Get() { return **_data; }
    const T& Get() const { return **_data; }
private:
    Resource& operator=(const Resource& other) = delete;
};

#endif // RESOURCE_H
