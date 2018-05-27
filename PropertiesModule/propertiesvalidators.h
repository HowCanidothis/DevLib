#ifndef PROPERTIESVALIDATORS_H
#define PROPERTIESVALIDATORS_H

#include <functional>
#include <QVariant>

class PropertiesValidators
{
    typedef std::function<void (const QVariant& property, QVariant& new_value)> FValidator;
public:
    PropertiesValidators();

    template<typename T>
    static FValidator OddValidator();
};

template<typename T>
PropertiesValidators::FValidator PropertiesValidators::OddValidator()
{
    return [](const QVariant& old, QVariant& v) {
        T new_value = v.value<T>();
        if(!(new_value % 2)) {
            if(new_value < old.value<T>()) {
                v = new_value - 1;
            }
            else {
                v = new_value + 1;
            }
        }
    };
}

#endif // PROPERTIESVALIDATORS_H
