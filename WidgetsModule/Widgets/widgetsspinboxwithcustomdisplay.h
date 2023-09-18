#ifndef WIDGETSSPINBOXWITHCUSTOMDISPLAY_H
#define WIDGETSSPINBOXWITHCUSTOMDISPLAY_H

#include <QSpinBox>

#include <PropertiesModule/internal.hpp>

class WidgetsSpinBoxWithCustomDisplay : public QSpinBox
{
    Q_OBJECT
    using Super = QSpinBox;
public:
    using ValueFromTextHandler = std::function<qint32 (const WidgetsSpinBoxWithCustomDisplay* spinBox, const QString&)>;
    using TextFromValueHandler = std::function<QString (const WidgetsSpinBoxWithCustomDisplay* spinBox, qint32)>;
    WidgetsSpinBoxWithCustomDisplay(QWidget* parent = nullptr);

    void Update();

    static const ValueFromTextHandler& GetDefaultValueFromTextHandler();
    static const TextFromValueHandler& GetDefaultTextFromValueHandler();

    void SetHandlers(const TextFromValueHandler& textFromValueHandler, const ValueFromTextHandler& valueFromTextHandler);

    void SetTextFromValueHandler(const TextFromValueHandler& textFromValueHandler);
    void SetValueFromTextHandler(const ValueFromTextHandler& valueFromTextHandler);

    DispatcherConnection MakeOptional(LocalPropertyBool* valid);
    void SetText(const QString& text);

private:
    QString textFromValue(int val) const override;
    qint32 valueFromText(const QString& text) const override;
    QValidator::State validate(QString& input, int& pos) const override;

private:
    TextFromValueHandler m_textFromValueHandler;
    ValueFromTextHandler m_valueFromTextHandler;
};

class WidgetsDoubleSpinBoxWithCustomDisplay : public QDoubleSpinBox
{
    Q_OBJECT
    using Super = QDoubleSpinBox;
public:
    using ValueFromTextHandler = std::function<double (const WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, const QString&)>;
    using TextFromValueHandler = std::function<QString (const WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, double)>;
    WidgetsDoubleSpinBoxWithCustomDisplay(QWidget* parent = nullptr);

    void Update();

    static const ValueFromTextHandler& GetDefaultValueFromTextHandler();
    static const TextFromValueHandler& GetDefaultTextFromValueHandler();

    void SetHandlers(const TextFromValueHandler& textFromValueHandler, const ValueFromTextHandler& valueFromTextHandler);
    void SetTextFromValueHandler(const TextFromValueHandler& textFromValueHandler);
    void SetValueFromTextHandler(const ValueFromTextHandler& valueFromTextHandler);

    DispatcherConnection MakeOptional(LocalPropertyBool* valid);
    void MakeOptional();

    void SetText(const QString& text);
    bool IsValid() const;

private:
    QString textFromValue(double val) const override;
    double valueFromText(const QString& text) const override;
    QValidator::State validate(QString& input, int& pos) const override;

private:
    TextFromValueHandler m_textFromValueHandler;
    ValueFromTextHandler m_valueFromTextHandler;
};

#endif // WIDGETSSPINBOXWITHCUSTOMDISPLAY_H
