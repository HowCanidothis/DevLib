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
    , m_textFromValueHandler([](const WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, double value) -> QString { return QString::number(value, 'f', spinBox->decimals()); })
    , m_valueFromTextHandler([](const WidgetsDoubleSpinBoxWithCustomDisplay* spin, const QString& text) -> double { return ::clamp(text.toDouble(), spin->minimum(), spin->maximum()); })
{}

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
