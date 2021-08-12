#include "textconvertercontext.h"

const TextConverterContext& TextConverterContext::DefaultContext()
{
    static TextConverterContext result;
    return result;
}
