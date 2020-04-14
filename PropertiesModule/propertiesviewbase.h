#ifndef PROPERTIESVIEWBASE_H
#define PROPERTIESVIEWBASE_H

#include <QColor>

class PropertiesViewBase
{
public:
    PropertiesViewBase();

protected:
    virtual class PropertiesDelegate* propertiesDelegate() const = 0;

protected:
    void setLeftGradientColor(const QColor& color);
    void setRightGradientColor(const QColor& color);
    void setRightGradientBorder(double border);

    const QColor& getLeftGradientColor() const;
    const QColor& getRightGradientColor() const;
    double getRightGradientBorder() const;
};

#endif // PROPERTIESVIEWBASE_H
