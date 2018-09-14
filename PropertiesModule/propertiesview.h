#ifndef PROPERTIESVIEW_H
#define PROPERTIESVIEW_H

#ifndef NO_WIDGETS_INTERFACE

#include <QTreeView>
#include "propertypromise.h"

class _Export PropertiesView : public QTreeView
{
    Q_OBJECT
    typedef QTreeView Super;
    Q_PROPERTY(QColor gradientLeft READ getLeftGradientColor WRITE setLeftGradientColor)
    Q_PROPERTY(QColor gradientRight READ getRightGradientColor WRITE setRightGradientColor)
    Q_PROPERTY(double gradientRightBorder READ getRightGradientBorder WRITE setRightGradientBorder)
public:
    PropertiesView(QWidget* parent=0, Qt::WindowFlags flags=0);
    PropertiesView(qint32 contextIndex, QWidget* parent=0, Qt::WindowFlags flags=0);

    void SetContextIndex(qint32 contextIndex);
    qint32 GetContextIndex() const;

    void Save(const QString& fileName);
    void Load(const QString& fileName);

    class PropertiesModel* GetPropertiesModel() const { return _propertiesModel; }

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    void validateActionsVisiblity();

protected:
    class PropertiesModel* _propertiesModel;

private:
    QModelIndex _indexUnderCursor;
    QAction* _actionOpenWithTextEditor;

    StringPropertyPtr _defaultTextEditor;

private:
    void setLeftGradientColor(const QColor& color);
    void setRightGradientColor(const QColor& color);
    void setRightGradientBorder(double border);

    const QColor& getLeftGradientColor() const;
    const QColor& getRightGradientColor() const;
    double getRightGradientBorder() const;

    class PropertiesDelegate* propertiesDelegate() const { return reinterpret_cast<PropertiesDelegate*>(itemDelegate()); }
};

#endif

#endif // PROPERTIESVIEW_H
