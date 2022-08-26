#ifndef PROPERTIESVIEW_H
#define PROPERTIESVIEW_H

#include <QTreeView>
#include <PropertiesModule/internal.hpp>

#include "propertiesviewbase.h"

class _Export PropertiesView : public QTreeView, public PropertiesViewBase
{
    Q_OBJECT
    typedef QTreeView Super;
    Q_PROPERTY(QColor gradientLeft READ getLeftGradientColor WRITE setLeftGradientColor)
    Q_PROPERTY(QColor gradientRight READ getRightGradientColor WRITE setRightGradientColor)
    Q_PROPERTY(double gradientRightBorder READ getRightGradientBorder WRITE setRightGradientBorder)
public:
    PropertiesView(QWidget* parent=0, Qt::WindowFlags flags=0);
    PropertiesView(const PropertiesScopeName& contextIndex, QWidget* parent=0, Qt::WindowFlags flags=0);

    void Update(const FAction& action);

    void SetContextIndex(const Name& contextIndex);
    const Name& GetContextIndex() const;

    void Save(const QString& fileName);
    void Load(const QString& fileName);

    class PropertiesModel* GetPropertiesModel() const { return m_propertiesModel; }

protected:
    PropertiesDelegate* propertiesDelegate() const override { return reinterpret_cast<PropertiesDelegate*>(itemDelegate()); }

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    void validateActionsVisiblity();

protected:
    class PropertiesModel* m_propertiesModel;

private:
    QModelIndex m_indexUnderCursor;
    QAction* m_actionOpenWithTextEditor;

    StringPropertyPtr m_defaultTextEditor;
};

#endif // PROPERTIESVIEW_H
