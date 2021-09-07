#include "widgetsspinboxwithcustomdisplay.h"

WidgetsSpinBoxWithCustomDisplay::WidgetsSpinBoxWithCustomDisplay(QWidget* parent)
    : Super(parent)
    , m_textFromValueHandler([](const WidgetsSpinBoxWithCustomDisplay*, qint32 value) -> QString { return QString::number(value); })
    , m_valueFromTextHandler([](const WidgetsSpinBoxWithCustomDisplay* spin, const QString& text) -> qint32 { return ::clamp(text.toInt(), spin->minimum(), spin->maximum()); })
{}

QString WidgetsSpinBoxWithCustomDisplay::textFromValue(int val) const
{
    return m_textFromValueHandler(this, val);
}

qint32 WidgetsSpinBoxWithCustomDisplay::valueFromText(const QString& text) const
{
    return m_valueFromTextHandler(this, text);
}

QValidator::State WidgetsSpinBoxWithCustomDisplay::validate(QString& input, int&) const
{
    static QRegExp regExp(R"((\d+\.?\d*))");

    if(input.isEmpty() || (input.size() == 1 && input.startsWith("-"))) {
        return QValidator::Intermediate;
    }

    QString inputCopy = input;
    if(input.startsWith("-")) {
        inputCopy = input.mid(1);
    }

    if(regExp.exactMatch(inputCopy)) {
        return QValidator::Acceptable;
    }
    return QValidator::Invalid;
}


WidgetsDoubleSpinBoxWithCustomDisplay::WidgetsDoubleSpinBoxWithCustomDisplay(QWidget* parent)
    : Super(parent)
    , m_textFromValueHandler(GetDefaultTextFromValueHandler())
    , m_valueFromTextHandler(GetDefaultValueFromTextHandler())
{}

const WidgetsDoubleSpinBoxWithCustomDisplay::ValueFromTextHandler& WidgetsDoubleSpinBoxWithCustomDisplay::GetDefaultValueFromTextHandler()
{
    static ValueFromTextHandler result = [](const WidgetsDoubleSpinBoxWithCustomDisplay* spin, const QString& text) -> double { return ::clamp(text.toDouble(), spin->minimum(), spin->maximum()); };
    return result;
}

const WidgetsDoubleSpinBoxWithCustomDisplay::TextFromValueHandler& WidgetsDoubleSpinBoxWithCustomDisplay::GetDefaultTextFromValueHandler()
{
    static TextFromValueHandler result = [](const WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, double value) -> QString { return QString::number(value, 'f', spinBox->decimals()); };
    return result;
}

DispatcherConnection WidgetsDoubleSpinBoxWithCustomDisplay::MakeOptional(LocalPropertyBool* valid)
{
    SetTextFromValueHandler([valid](const WidgetsDoubleSpinBoxWithCustomDisplay* spin, double value) -> QString {
        if(!*valid) {
            return "-";
        }
        return GetDefaultTextFromValueHandler()(spin, value);
    });

    return valid->OnChange.Connect(this, [this]{
        setDecimals(decimals());
    });
}

QString WidgetsDoubleSpinBoxWithCustomDisplay::textFromValue(double val) const
{
    return m_textFromValueHandler(this, val);
}

double WidgetsDoubleSpinBoxWithCustomDisplay::valueFromText(const QString& text) const
{
    return m_valueFromTextHandler(this, text);
}

QValidator::State WidgetsDoubleSpinBoxWithCustomDisplay::validate(QString& input, int&) const
{
    static QRegExp regExp(R"((\d+\.?\d*))");
    
    if(input.isEmpty() || (input.size() == 1 && input.startsWith("-")) || input.startsWith(".") || input.startsWith(",")) {
        return QValidator::Intermediate;
    }

    QString inputCopy = input;
    if(input.startsWith("-")) {
        inputCopy = input.mid(1);
    }

    if(regExp.exactMatch(inputCopy)) {
        return QValidator::Acceptable;
    }
    return QValidator::Invalid;
}
