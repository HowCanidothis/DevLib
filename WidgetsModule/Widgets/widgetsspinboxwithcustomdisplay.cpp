#include "widgetsspinboxwithcustomdisplay.h"

WidgetsSpinBoxWithCustomDisplay::WidgetsSpinBoxWithCustomDisplay(QWidget* parent)
    : Super(parent)
    , m_textFromValueHandler(GetDefaultTextFromValueHandler())
    , m_valueFromTextHandler(GetDefaultValueFromTextHandler())
    , m_emptyInputIsValid(false)
{}

const WidgetsSpinBoxWithCustomDisplay::ValueFromTextHandler& WidgetsSpinBoxWithCustomDisplay::GetDefaultValueFromTextHandler()
{
    static ValueFromTextHandler result = [](const WidgetsSpinBoxWithCustomDisplay* spin, const QString& text) -> qint32 { return ::clamp(text.toInt(), spin->minimum(), spin->maximum()); };
    return result;
}

const WidgetsSpinBoxWithCustomDisplay::TextFromValueHandler& WidgetsSpinBoxWithCustomDisplay::GetDefaultTextFromValueHandler()
{
    static TextFromValueHandler result = [](const WidgetsSpinBoxWithCustomDisplay*, qint32 value) -> QString { return QString::number(value); };
    return result;
}

QString WidgetsSpinBoxWithCustomDisplay::textFromValue(int val) const
{
    return m_textFromValueHandler(this, val);
}

DispatcherConnection WidgetsSpinBoxWithCustomDisplay::MakeOptional(LocalPropertyBool* valid)
{
    SetTextFromValueHandler([valid](const WidgetsSpinBoxWithCustomDisplay* spin, double value) -> QString {
        if(!*valid) {
            return "-";
        }
        return GetDefaultTextFromValueHandler()(spin, value);
    });

    SetValueFromTextHandler([this, valid](const WidgetsSpinBoxWithCustomDisplay* spin, const QString& text) -> double {
        if(text.isEmpty()) {
            *valid = false;
            return value();
        }
        return GetDefaultValueFromTextHandler()(spin, text);
    });

    m_emptyInputIsValid = true;
    auto updateDisplay = [this]{
        setDisplayIntegerBase(displayIntegerBase());
    };
    auto result = valid->OnChange.Connect(this, updateDisplay);
    updateDisplay();
    return result;
}

qint32 WidgetsSpinBoxWithCustomDisplay::valueFromText(const QString& text) const
{
    return m_valueFromTextHandler(this, text);
}

QValidator::State WidgetsSpinBoxWithCustomDisplay::validate(QString& input, int&) const
{
    static QRegExp regExp(R"((\d+\.?\d*))");

    if(input.isEmpty() || (input.size() == 1 && input.startsWith("-"))) {
        return m_emptyInputIsValid ? QValidator::Acceptable : QValidator::Intermediate;
    }

    if(input.size() == 1 && input.startsWith("-")) {
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
    , m_emptyInputIsValid(true)
{}

static QRegExp regExpFractial(R"((\d+)(\s)(\d+)?(\/)?(\d+)?)");

const WidgetsDoubleSpinBoxWithCustomDisplay::ValueFromTextHandler& WidgetsDoubleSpinBoxWithCustomDisplay::GetDefaultValueFromTextHandler()
{
    static ValueFromTextHandler result = [](const WidgetsDoubleSpinBoxWithCustomDisplay* spin, const QString& text) -> double {
        double value;
        if(regExpFractial.indexIn(text) != -1) {
            auto fractial = regExpFractial.cap(5).toDouble();
            value = regExpFractial.cap(1).toDouble() + (fuzzyIsNull(fractial) ? 0.0 : (regExpFractial.cap(3).toDouble() / fractial));
        } else {
            value = text.toDouble();
        }
        return ::clamp(value, spin->minimum(), spin->maximum());
    };
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

    SetValueFromTextHandler([this, valid](const WidgetsDoubleSpinBoxWithCustomDisplay* spin, const QString& text) -> double {
        if(text.isEmpty()) {
            *valid = false;
            return value();
        }
        valid->EditSilent() = true;
        return GetDefaultValueFromTextHandler()(spin, text);
    });

    m_emptyInputIsValid = true;
    auto updateDisplay = [this]{
        setDecimals(decimals());
    };
    auto result = valid->OnChange.Connect(this, updateDisplay);
    updateDisplay();
    return result;
}

void WidgetsDoubleSpinBoxWithCustomDisplay::MakeOptional()
{
    auto property = ::make_shared<LocalPropertyBool>(true);
    MakeOptional(property.get());
    setProperty("IsValid", QVariant::fromValue(property));
}

bool WidgetsDoubleSpinBoxWithCustomDisplay::IsValid() const
{
    auto value = property("IsValid");
    return !value.isValid() ? true : value.value<SharedPointer<LocalPropertyBool>>()->Native();
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

    if(input.isEmpty()) {
        return m_emptyInputIsValid ? QValidator::Acceptable : QValidator::Intermediate;
    }

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

    if(regExpFractial.indexIn(input) != -1) {
        return regExpFractial.cap(5).isEmpty() ? QValidator::Intermediate : QValidator::Acceptable;
    }

    return QValidator::Invalid;
}
