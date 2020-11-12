#ifndef UTILS_H
#define UTILS_H

#include <functional>

#ifdef QT_GUI_LIB

#include <QAbstractItemModel>
#include "SharedModule/internal.hpp"

_Export void forEachModelIndex(QAbstractItemModel* model, QModelIndex parent, const std::function<void (const QModelIndex& index)>& function);
_Export class QAction* createAction(const QString& title, const std::function<void ()>& handle, QWidget* menu);
_Export class QAction* createAction(const QString &title, const std::function<void (QAction*)> &handle, QWidget* menu);
_Export class QAction* createCheckboxAction(const QString& title, bool checked, const std::function<void (bool)>& handler, QWidget* menu);
_Export class QAction* createColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler, QWidget* menu);
_Export class QAction* createDoubleAction(const QString& title, double value, const std::function<void (double value)>& handler, QWidget* menu);
_Export class QMenu* createPreventedFromClosingMenu(const QString& title, QMenu* menu = nullptr);
using QtEventFilterHandler = std::function<bool (QObject *watched, QEvent *event)>;
_Export void installEventFilter(QObject* target, const QtEventFilterHandler& eventFilter);

#endif

#endif // UTILS_H
