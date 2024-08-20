#ifndef WIDGETSPICTUREPICKER_H
#define WIDGETSPICTUREPICKER_H

#include <QLabel>

#include <PropertiesModule/internal.hpp>
#include <PropertiesModule/Ui/internal.hpp>

class WidgetsPicturePicker : public QLabel
{
    Q_OBJECT
    using Super = QLabel;
public:
    explicit WidgetsPicturePicker(QWidget* parent = nullptr);

    void Initialize(LocalPropertiesWidgetConnectorsContainer& connectors, const FTranslationHandler& t = TR_NONE, const QString& forceDefaultDir = QString());

    LocalProperty<QImage> Image;
    LocalPropertyString ImagePath;

    // QWidget interface
protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    class QPushButton* m_deleteButton;
    class QPushButton* m_addButton;
};

#endif // WIDGETSPICTUREPICKER_H
