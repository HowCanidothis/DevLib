#ifndef TEXTCONVERTERCONTEXT_H
#define TEXTCONVERTERCONTEXT_H

struct TextConverterContext
{
    qint32 FloatPrecision = 7;
    qint32 DoublePrecision = 14;

    static const TextConverterContext& DefaultContext();
};

#endif // TEXTCONVERTERCONTEXT_H
