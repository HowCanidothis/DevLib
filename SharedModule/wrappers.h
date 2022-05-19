#ifndef SHARED_LIB_WRAPPERS_H
#define SHARED_LIB_WRAPPERS_H

template<class T, typename ComparisonTarget = T, typename Container = QVector<T>>
class ContainerWrapper
{
public:
    using value_type = T;
    using const_iterator = typename Container::const_iterator;
    using iterator = typename Container::iterator;
    using FIdGetter = std::function<const ComparisonTarget& (const value_type&)>;
    using FLessThan = std::function<bool (const value_type&, const value_type&)>;

    ContainerWrapper(Container* container, const FIdGetter& idGetter = [](const T& value) -> const T& { return value; })
        : m_container(container)
        , m_lessThan([this](const value_type& f, const value_type& s) -> bool {
            return m_idGetter(f) < m_idGetter(s);
        })
        , m_idGetter(idGetter)
    {}

    size_t IndexOf(const const_iterator& iterator) const
    {
        return std::distance(m_container->cbegin(), iterator);
    }

    ContainerWrapper& Insert(const Container& container)
    {
        for(const auto& object : container) {
            Insert(object);
        }
        return *this;
    }

    ContainerWrapper& Sort()
    {
        std::sort(m_container->begin(), m_container->end(), m_lessThan);
        return *this;
    }

    ContainerWrapper& Insert(const value_type& object)
    {
        auto foundIt = find(m_idGetter(object));
        m_container->insert(foundIt, object);
        return *this;
    }

    const value_type& FindValue(const ComparisonTarget& id) const
    {
        static value_type defaultValue;
        auto foundIt = Find(id);
        if(foundIt != m_container->end() && m_idGetter(*foundIt) == id) {
            return *foundIt;
        }
        return defaultValue;
    }

    void Foreach(const ComparisonTarget& id, const std::function<void (value_type&)>& handler)
    {
        auto firstElement = find(id);
        while(firstElement != m_container->end() && *firstElement == id) {
            handler(*firstElement);
            firstElement++;
        }
    }

    const_iterator Find(const ComparisonTarget& id) const
    {
        return std::lower_bound(m_container->begin(), m_container->end(), id, [this](const value_type& f, const ComparisonTarget& s){
            return m_idGetter(f) < s;
        });
    }
private:
    iterator find(const ComparisonTarget& id)
    {
        return std::lower_bound(m_container->begin(), m_container->end(), id, [this](const value_type& f, const ComparisonTarget& s){
            return m_idGetter(f) < s;
        });
    }

private:
    Container* m_container;
    FLessThan m_lessThan;
    FIdGetter m_idGetter;
};

#endif // WRAPPERS_H
