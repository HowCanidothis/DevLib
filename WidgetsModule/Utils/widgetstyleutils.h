#ifndef WIDGETS_STYLE_UTILS_H
#define WIDGETS_STYLE_UTILS_H

#include <QVariant>

namespace StyleUtils {
    // Qt bug? Widgets which don't participate at layout system does not update their padding
    // SizeAdjuster fixes this issue
    void InstallSizeAdjuster(QWidget* widget);
}

#endif // POLISHER_H
