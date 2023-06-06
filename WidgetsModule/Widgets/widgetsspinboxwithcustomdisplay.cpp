#include "widgetsspinboxwithcustomdisplay.h"

#include <QLineEdit>

static const char* IsValidPropertyName = "IsValid";

WidgetsSpinBoxWithCustomDisplay::WidgetsSpinBoxWithCustomDisplay(QWidget* parent)
    : Super(parent)
    , m_textFromValueHandler(GetDefaultTextFromValueHandler())
    , m_valueFromTextHandler(GetDefaultValueFromTextHandler())
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
    setProperty(IsValidPropertyName, (size_t)valid);
    SetTextFromValueHandler([valid](const WidgetsSpinBoxWithCustomDisplay* spin, double value) -> QString {
        if(!*valid) {
            return QString();
        }
        return GetDefaultTextFromValueHandler()(spin, value);
    });

    lineEdit()->setPlaceholderText(DASH);

    SetValueFromTextHandler([this, valid](const WidgetsSpinBoxWithCustomDisplay* spin, const QString& text) -> double {
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
        if(!hasFocus()) {
            setPrefix(prefix());
        }
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
    static TextFromValueHandler result = [](const WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, double value) -> QString { return LanguageSettings::DoubleToString(value, spinBox->decimals()); };
    return result;
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
        if(!hasFocus()) {
            setPrefix(prefix());
        }
    };
    auto result = valid->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, updateDisplay);
    return result;
}

void WidgetsDoubleSpinBoxWithCustomDisplay::MakeOptional()
{
    auto property = ::make_shared<LocalPropertyBool>(true);
    setProperty("IsValidStorage", qVariantFromValue(property));
    MakeOptional(property.get());
}

void WidgetsDoubleSpinBoxWithCustomDisplay::SetText(const QString& text)
{
    auto value = property(IsValidPropertyName);
    setValue(text.toDouble());
    if(!value.isValid()) {
        return;
    }
    *((LocalPropertyBool*)value.toLongLong()) = !text.isEmpty();
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
