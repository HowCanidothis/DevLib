#include "flowwidget.h"

#include "flowlayout.h"

FlowWidget::FlowWidget(QWidget* parent)
    : Super(parent)
{
    QWidget* contentWidget = new QWidget;
    contentWidget->setLayout(m_layout = new FlowLayout);
    setWidget(contentWidget);
    setWidgetResizable(true);

    HSpacing.EditSilent() = m_layout->HorizontalSpacing();
    VSpacing.EditSilent() = m_layout->VerticalSpacing();

    HSpacing.Subscribe([this]{
        m_layout->SetHorizontalSpacing(HSpacing);
    });
    VSpacing.Subscribe([this]{
        m_layout->SetVerticalSpacing(VSpacing);
    });
}

void FlowWidget::AddWidget(const Name& tag, QWidget* widget)
{
    m_taggedWidgets.insert(tag, widget);
    AddWidget(widget);
}

void FlowWidget::RemoveWidgets(const QSet<Name>& tags)
{
    QSet<QWidget*> widgets;
    for(const auto& tag : tags) {
        auto foundIt = m_taggedWidgets.find(tag);
        if(foundIt != m_taggedWidgets.end()) {
            widgets.insert(foundIt.value());
        }
    }
    m_layout->RemoveWidgets(widgets);
    for(auto* widget : widgets) {
        widget->deleteLater();
    }
}

void FlowWidget::AddWidget(QWidget* widget)
{
    m_layout->addWidget(widget);
}

void FlowWidget::RemoveWidget(QWidget* widget)
{
    m_layout->RemoveWidgets({widget});
}
