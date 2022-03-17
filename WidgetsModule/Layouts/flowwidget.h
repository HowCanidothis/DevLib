#ifndef FLOWWIDGET_H
#define FLOWWIDGET_H

#include <QScrollArea>

#include <PropertiesModule/internal.hpp>

class FlowWidget : public QScrollArea
{
    Q_OBJECT
    using Super = QScrollArea;
    Q_PROPERTY(qint32 hSpacing MEMBER HSpacing);
    Q_PROPERTY(qint32 vSpacing MEMBER VSpacing);

public:
    FlowWidget(QWidget* parent = nullptr);

    void Clear();

    void AddWidget(const Name& tag, QWidget* widget);
    void RemoveWidgets(const QSet<Name>& tags);

    void AddWidget(QWidget* widget);
    void RemoveWidget(QWidget* widget);

    LocalPropertyInt HSpacing;
    LocalPropertyInt VSpacing;

private:
    class FlowLayout* m_layout;
    QHash<Name, QWidget*> m_taggedWidgets;
};
#endif // FLOWWIDGET_H
