#include "widgetsspinboxwithcustomdisplay.h"

#include <QLineEdit>

static const char* IsValidPropertyName = "IsValid";

WidgetsSpinBoxWithCustomDisplay::WidgetsSpinBoxWithCustomDisplay(QWidget* parent)
    : Super(parent)
    , m_textFromValueHandler(GetDefaultTextFromValueHandler())
    , m_valueFromTextHandler(GetDefaultValueFromTextHandler())
{}

void WidgetsSpinBoxWithCustomDisplay::Update()
{
    if(!hasFocus()) {
        setPrefix(prefix());
    }
}

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

void WidgetsSpinBoxWithCustomDisplay::SetHandlers(const TextFromValueHandler& textFromValueHandler, const ValueFromTextHandler& valueFromTextHandler)
{
    m_textFromValueHandler = textFromValueHandler;
    m_valueFromTextHandler = valueFromTextHandler;
}

void WidgetsSpinBoxWithCustomDisplay::SetTextFromValueHandler(const TextFromValueHandler& textFromValueHandler)
{
    m_textFromValueHandler = textFromValueHandler;
}

void WidgetsSpinBoxWithCustomDisplay::SetValueFromTextHandler(const ValueFromTextHandler& valueFromTextHandler)
{
    m_valueFromTextHandler = valueFromTextHandler;
}

QString WidgetsSpinBoxWithCustomDisplay::textFromValue(int val) const
{
    return m_textFromValueHandler(this, val);
}

DispatcherConnection WidgetsSpinBoxWithCustomDisplay::MakeOptional(LocalPropertyBool* valid)
{
    setProperty(IsValidPropertyName, (size_t)valid);
    SetTextFromValueHandler([valid](const WidgetsSpinBoxWithCustomDisplay* spin, double value) -> QString {
        if(!*valid) {
            return QString();
        }
        return GetDefaultTextFromValueHandler()(spin, value);
    });

    lineEdit()->setPlaceholderText(DASH);

    SetValueFromTextHandler([this, valid](const WidgetsSpinBoxWithCustomDisplay* spin, const QString& text) -> double {
        if(property(WidgetProperties::ForceDisabled).toBool()) {
            return spin->value();
        }
        if(text.isEmpty()) {
            *valid = false;
            return value();
        } else {
            ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[valid]{
                *valid = true;
            });
        }
        return GetDefaultValueFromTextHandler()(spin, text);
    });

    auto updateDisplay = [this]{
        Update();
    };
    auto result = valid->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, updateDisplay);
    return result;
}

void WidgetsSpinBoxWithCustomDisplay::SetText(const QString& text)
{
    auto value = property(IsValidPropertyName);
    setValue(text.toDouble());
    if(!value.isValid()) {
        return;
    }
    *((LocalPropertyBool*)value.toLongLong()) = !text.isEmpty();
}

qint32 WidgetsSpinBoxWithCustomDisplay::valueFromText(const QString& text) const
{
    return m_valueFromTextHandler(this, text);
}

thread_local static QRegExp regExpFloating(R"(\s*(\d+)[\.\,]?(\d*)\s*)");

QValidator::State WidgetsSpinBoxWithCustomDisplay::validate(QString& input, int&) const
{
    if(input.isEmpty()) {
        return QValidator::Acceptable;
    }

    if((input.size() == 1 && input.startsWith(DASH))) {
        return QValidator::Intermediate;
    }

    QString inputCopy = input.replace(',', '.');
    if(input.startsWith(DASH)) {
        inputCopy = input.mid(1);
    }

    if(regExpFloating.exactMatch(inputCopy)) {
        return QValidator::Acceptable;
    }
    return QValidator::Invalid;
}


WidgetsDoubleSpinBoxWithCustomDisplay::WidgetsDoubleSpinBoxWithCustomDisplay(QWidget* parent)
    : Super(parent)
    , m_textFromValueHandler(GetDefaultTextFromValueHandler())
    , m_valueFromTextHandler(GetDefaultValueFromTextHandler())
{}

void WidgetsDoubleSpinBoxWithCustomDisplay::Update()
{
    if(!hasFocus()) {
        setPrefix(prefix());
    }
}

void WidgetsDoubleSpinBoxWithCustomDisplay::SetValue(const std::optional<double>& val)
{
    auto value = property(IsValidPropertyName);
    setValue(val.value_or(0.0));
    if(!value.isValid()) {
        return;
    }
    *((LocalPropertyBool*)value.toLongLong()) = val.has_value();
}

thread_local static QRegExp regExpFractial(R"(([-+])?(\d+)\s*(\d+)?\s*(\/)?\s*(\d+)?)");

const WidgetsDoubleSpinBoxWithCustomDisplay::ValueFromTextHandler& WidgetsDoubleSpinBoxWithCustomDisplay::GetDefaultValueFromTextHandler()
{
    static ValueFromTextHandler result = [](const WidgetsDoubleSpinBoxWithCustomDisplay* spin, const QString& text) -> double {
        double value;
        if(regExpFractial.indexIn(text) != -1 && !regExpFractial.cap(5).isEmpty()) {
            auto fractial = regExpFractial.cap(5);
            auto meaning = regExpFractial.cap(3);
            auto main = regExpFractial.cap(2);
            if(meaning.isEmpty()) {
                auto fractialValue = fractial.toDouble();
                value = main.toDouble() / (fuzzyIsNull(fractialValue) ? 1.0 : fractialValue);
            } else {
                auto fractialValue = fractial.toDouble();
                value = main.toDouble() + (fuzzyIsNull(fractialValue) ? 0.0 : (meaning.toDouble() / fractialValue));
            }
            if(regExpFractial.cap(1) == DASH) {
                value = -value;
            }
        } else if(regExpFloating.indexIn(text) != -1){
            value = QString("%1.%2").arg(regExpFloating.cap(1), regExpFloating.cap(2)).toDouble();
            if(regExpFractial.cap(1) == DASH) {
                value = -value;
            }
        } else {
            value = text.toDouble();
        }
        return ::clamp(value, spin->minimum(), spin->maximum());
    };
    return result;
}

const WidgetsDoubleSpinBoxWithCustomDisplay::TextFromValueHandler& WidgetsDoubleSpinBoxWithCustomDisplay::GetDefaultTextFromValueHandler()
{
    static TextFromValueHandler result = [](const WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, double value) -> QString { return SharedSettings::IsInitialized() ? LanguageSettings::DoubleToString(value, spinBox->decimals()) : QString::number(value, 'f', spinBox->decimals()); };
    return result;
}

void WidgetsDoubleSpinBoxWithCustomDisplay::SetHandlers(const TextFromValueHandler& textFromValueHandler, const ValueFromTextHandler& valueFromTextHandler)
{
    m_textFromValueHandler = textFromValueHandler;
    m_valueFromTextHandler = valueFromTextHandler;
}

void WidgetsDoubleSpinBoxWithCustomDisplay::SetTextFromValueHandler(const TextFromValueHandler& textFromValueHandler)
{
    m_textFromValueHandler = textFromValueHandler;
}

void WidgetsDoubleSpinBoxWithCustomDisplay::SetValueFromTextHandler(const ValueFromTextHandler& valueFromTextHandler)
{
    m_valueFromTextHandler = valueFromTextHandler;
}

DispatcherConnection WidgetsDoubleSpinBoxWithCustomDisplay::MakeOptional(LocalPropertyBool* valid)
{
    setProperty(IsValidPropertyName, (size_t)valid);
    SetTextFromValueHandler([valid](const WidgetsDoubleSpinBoxWithCustomDisplay* spin, double value) -> QString {
        if(!*valid) {
            return QString();
        }
        return GetDefaultTextFromValueHandler()(spin, value);
    });

    lineEdit()->setPlaceholderText(DASH);

    SetValueFromTextHandler([this, valid](const WidgetsDoubleSpinBoxWithCustomDisplay* spin, const QString& text) -> double {
        if(property(WidgetProperties::ForceDisabled).toBool()) {
            return spin->value();
        }
        if(text.isEmpty()) {
            *valid = false;
            return value();
        }
        ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[valid]{
            *valid = true;
        });
        return GetDefaultValueFromTextHandler()(spin, text);
    });

    auto updateDisplay = [this]{
        Update();
    };
    auto result = valid->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, updateDisplay);
    return result;
}

void WidgetsDoubleSpinBoxWithCustomDisplay::MakeOptional()
{
    auto property = ::make_shared<LocalPropertyBool>(true);
    setProperty("IsValidStorage", QVariant::fromValue(property));
    MakeOptional(property.get());
}

void WidgetsDoubleSpinBoxWithCustomDisplay::SetText(const QString& text)
{
    if(text.isEmpty()) {
        SetValue(std::nullopt);
        return;
    }
    SetValue(text.toDouble());
}

bool WidgetsDoubleSpinBoxWithCustomDisplay::IsValid() const
{
    auto value = property(IsValidPropertyName);
    return !value.isValid() ? true : ((LocalPropertyBool*)value.toLongLong())->Native();
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
    if(input.isEmpty()) {
        return QValidator::Acceptable;
    }

    if((input.size() == 1 && (input.startsWith(DASH) || input.startsWith("+"))) || input.startsWith(".") || input.startsWith(",")) {
        return QValidator::Intermediate;
    }

    QString inputCopy = input;
    inputCopy.replace(',', '.');
    if(input.startsWith(DASH) || input.startsWith("+")) {
        inputCopy = input.mid(1);
    }

    if(regExpFloating.exactMatch(inputCopy)) {
        return QValidator::Acceptable;
    }

    if(regExpFractial.indexIn(input) != -1) {
        return regExpFractial.cap(5).isEmpty() ? QValidator::Intermediate : QValidator::Acceptable;
    }

    return QValidator::Invalid;
}
