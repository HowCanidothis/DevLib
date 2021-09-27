#ifndef WPSCIMPORTPREPARATIONDIALOG_H
#define WPSCIMPORTPREPARATIONDIALOG_H

#include <QWidget>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsImportView;
}

enum class LocaleType {
    English,
    Russian,
    First = English,
    Last = Russian,
};

template<>
struct EnumHelper<LocaleType>
{
    static QStringList GetNames() { return { tr("EN"), tr("RU")}; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

enum class GroupKeyboardSeparator {
    Space,
    Semicolon,
    Tab,
    Hash,
    Pipe,
    Comma,
    First = Space,
    Last = Comma,
};

template<>
struct EnumHelper<GroupKeyboardSeparator>
{
    static QStringList GetNames() { return { tr("Space"), ";", tr("Tab"), "#", "|", ","}; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

enum class DecimalKeyboardSeparator {
    Dot,
    Comma,
    First = Dot,
    Last = Comma,
};

template<>
struct EnumHelper<DecimalKeyboardSeparator>
{
    static QStringList GetNames() { return { ".", ","}; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

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
    Dispatcher OnTransited;

    void Initialize(const QList<QString>& data, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns = {});
    void Initialize(const QList<QVector<QVariant>>& data, QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns = {});
    void SetPreviewModel(QAbstractItemModel* model);

    QTableView* GetSourceTableView() const;
    QTableView* GetPreviewTableView() const;

    void SetVisibilityMode(VisibilityFlags mode);

    class WidgetsMatchingAttachment* GetAttachment() { return m_matchingAttachment.get(); }
	
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
