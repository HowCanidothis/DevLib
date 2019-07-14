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
        T newValue = v.value<T>();
        if(!(newValue % 2)) {
            if(newValue < old.value<T>()) {
                v = newValue - 1;
            }
            else {
                v = newValue + 1;
            }
        }
    };
}

#endif // PROPERTIESVALIDATORS_H
