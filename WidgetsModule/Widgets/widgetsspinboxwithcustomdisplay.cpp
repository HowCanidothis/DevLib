#include "widgetsspinboxwithcustomdisplay.h"

WidgetsSpinBoxWithCustomDisplay::WidgetsSpinBoxWithCustomDisplay(QWidget* parent)
    : Super(parent)
    , m_textFromValueHandler([this](const WidgetsSpinBoxWithCustomDisplay*, qint32 value) { return Super::textFromValue(value); })
    , m_valueFromTextHandler([this](const WidgetsSpinBoxWithCustomDisplay*, const QString& text) { return Super::valueFromText(text); })
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

    if(regExp.exactMatch(input)) {
        return QValidator::Acceptable;
    }
    return QValidator::Invalid;
}
