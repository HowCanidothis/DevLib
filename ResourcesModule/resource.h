#ifndef RESOURCE_H
#define RESOURCE_H

#include <Qt>
#include <functional>

class ResourceBase
{
    typedef std::function<void*()> LoadFunction;

    void** _data;
    qint32* _counter;
    LoadFunction loader;
public:
    ResourceBase(const LoadFunction& loader)
        : _data((void**)malloc(sizeof(size_t)))
        , _counter(new qint32)
        , loader(loader)
    {
        *_data = nullptr;
        *_counter = 0;
    }

    void setLoader(const LoadFunction& loader) {
        this->loader = loader;
    }

    void load() const{
        Q_ASSERT(*_data == nullptr);
        *_data = loader();
    }

    bool isNull() const {
         return *_data == nullptr;
    }

    bool isValid() const {
        return !isNull();
    }

    template<typename T> friend class Resource;
};

template<typename T>
class Resource
{
    T** _data;
    qint32* _counter;
    Resource() {}
public:
    ~Resource() {
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

    bool isNull() const { return *_data == nullptr; }
    const T* data() const { return *_data; }
    T* data() { return *_data; }
    T& get() { return **_data; }
    const T& get() const { return **_data; }
    void create(T* data) {
        Q_ASSERT(*_data == nullptr);
        *_data = data;
    }
private:
    Resource& operator=(const Resource& other) = delete;
};

#endif // RESOURCE_H
