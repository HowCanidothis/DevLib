#ifndef TEXTCONVERTERCONTEXT_H
#define TEXTCONVERTERCONTEXT_H

struct TextConverterContext
{
    qint32 FloatPrecision = 7;
    qint32 DoublePrecision = 14;
    const char* ImageConversionExtension = "PNG";

    TextConverterContext& SetImageConversionExtension(const char* ice) { ImageConversionExtension = ice; return *this; }
    TextConverterContext& SetFloatPrecision(qint32 precision) { FloatPrecision = precision; return *this; }
    TextConverterContext& SetDoublePrecision(qint32 precision) { DoublePrecision = precision; return *this; }

    static const TextConverterContext& DefaultContext();
};

#endif // TEXTCONVERTERCONTEXT_H
