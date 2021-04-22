#include "widgethelpers.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTableView>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>

void WidgetAppearance::SetVisibleAnimated(QWidget* widget, bool visible)
{
    if(widget->isVisibleTo(widget->parentWidget()) == visible) {
        return;
    }

    if(visible) {
        WidgetAppearance::ShowAnimated(widget);
    } else {
        WidgetAppearance::HideAnimated(widget);
    }
}

void WidgetAppearance::ShowAnimated(QWidget* widget)
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect,"opacity");
    animation->setDuration(2000);
    animation->setStartValue(0.0);
    animation->setEndValue(0.8);
    animation->setEasingCurve(QEasingCurve::InBack);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
    widget->show();
}

void WidgetAppearance::HideAnimated(QWidget* widget)
{
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect,"opacity");
    animation->setDuration(2000);
    animation->setStartValue(0.8);
    animation->setEndValue(0);
    animation->setEasingCurve(QEasingCurve::OutBack);
    animation->connect(animation, &QPropertyAnimation::finished, [widget]{
        widget->hide();
    });
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void WidgetContent::ForeachChildWidget(QWidget* target, const std::function<void (QWidget*)>& handler)
{
    auto childWidgets = target->findChildren<QWidget*>();
    for(auto* childWidget : childWidgets) {
        handler(childWidget);
    }
}

void WidgetContent::CopySelectedTableContentsToClipboard(QTableView* tableView)
{
    auto selectedIndexes = tableView->selectionModel()->selectedIndexes();
    if(selectedIndexes.isEmpty()) {
        return;
    }

    auto* header = tableView->horizontalHeader();
    std::sort(selectedIndexes.begin(), selectedIndexes.end(), [header](const QModelIndex& f, const QModelIndex& s) {
        if(f.row() == s.row()) {
            return header->visualIndex(f.column()) < header->visualIndex(s.column());
        }

        return f.row() < s.row();
    });

    QString text;
    auto rowIndex = selectedIndexes.first().row();
    for(const auto& index : selectedIndexes) {
        if(header->isSectionHidden(index.column())) {
            continue;
        }
        if(rowIndex != index.row()) {
            rowIndex = index.row();
            if(!text.isEmpty()) {
                text.resize(text.size() - 1);
            }
            text += "\n";
        }
        text += index.data().toString() + "\t";
    }
    if(!text.isEmpty()) {
        text.resize(text.size() - 1);
    }

    QClipboard* clipboard = qApp->clipboard();
    clipboard->setText(text);
}

