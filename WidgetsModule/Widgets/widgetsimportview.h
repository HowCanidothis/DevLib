#ifndef WPSCIMPORTPREPARATIONDIALOG_H
#define WPSCIMPORTPREPARATIONDIALOG_H

#include <QWidget>
#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

namespace Ui {
class WidgetsImportView;
}

class WidgetsImportView : public QWidget
{
	Q_OBJECT
    using Super = QWidget;
public:
    enum VisibilityFlag
    {
        VisibilityFlag_Locale = 0x1,
        VisibilityFlag_GroupSeparator = 0x2,
        VisibilityFlag_DecimalSeparator = 0x4,
        VisibilityFlag_Date = 0x8
    };

    DECL_FLAGS(VisibilityFlags, VisibilityFlag);

    explicit WidgetsImportView(QWidget *parent = nullptr);
    ~WidgetsImportView() override;
	
    LocalPropertyBool ShowPreview;
	LocalPropertySequentialEnum<GroupKeyboardSeparator> GroupSeparator;
	LocalPropertySequentialEnum<DecimalKeyboardSeparator> DecimalSeparator;
	LocalPropertyString DateTimeFormat;
    LocalProperty<QLocale> Locale;
    LocalPropertySequentialEnum<LocaleType> ImportLocale;
    CommonDispatcher<qint32, qint32> OnMatchingChanged;
    Dispatcher OnTransitionStarted;
    Dispatcher OnTransited;

    void Initialize(const QList<QString>& data, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns = {});
    void Initialize(const QList<QVector<QVariant>>& data, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns = {});
    void SetPreviewModel(QAbstractItemModel* model);

    class QTableView* GetSourceTableView() const;
    QTableView* GetPreviewTableView() const;

    void SetVisibilityMode(VisibilityFlags mode);

    class WidgetsMatchingAttachment* GetAttachment() { return m_matchingAttachment.get(); }
	
private slots:
    void on_BtnFlip_clicked();

private:
    class VariantListModel* GetModel() const;
    void initializeMatching(QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns);
	
private:
    Ui::WidgetsImportView *ui;
	DispatcherConnectionsSafe m_connections;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
    ScopedPointer<class WidgetsMatchingAttachment> m_matchingAttachment;
};

#endif // WPSCIMPORTPREPARATIONDIALOG_H
