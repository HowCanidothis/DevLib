#ifndef WIDGETSLAYOUTCOMPONENT_H
#define WIDGETSLAYOUTCOMPONENT_H

template<class T>
struct WidgetsLayoutComponent {
    T* Widget;

    WidgetsLayoutComponent(const QString& name)
        : Widget(new T())
    {
        Widget->setObjectName(name);
    }
    void Detach()
    {
        delete Widget;
    }
};

#endif // WIDGETSLAYOUTCOMPONENT_H
