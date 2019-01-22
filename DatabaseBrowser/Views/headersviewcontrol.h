#ifndef HEADERSVIEWCONTROL_H
#define HEADERSVIEWCONTROL_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class HeadersViewControl;
}

class HeadersViewControl : public QWidget
{
    Q_OBJECT

public:
    explicit HeadersViewControl(QWidget *parent = nullptr);
    ~HeadersViewControl();

Q_SIGNALS:
    void modelAboutToBeChanged();
    void modelChanged();

private slots:
    void on_BtnAdd_clicked();

    void on_BtnRemove_clicked();

    void onSelectionChanged();

    void on_BtnEdit_clicked();

private:
    Ui::HeadersViewControl *ui;

    PropertiesDialogGeometryProperty m_headersDialogGeometry;
    PropertiesDialogGeometryProperty m_FieldsDialogGeometry;
    PointerPropertyPtr<class DbDatabase> m_currentDatabase;
    PointerProperty<class DbTableHeader> m_currentHeader;
    struct MDbTableField* m_currentField;
    LocalPropertySet<QModelIndex> m_selectedRows;
    DbTableHeader* m_currentFieldHeader;

    class DbModelHeadsTree* model() const;
    bool hasCurrentFieldOrCurrentHeader() const;
    qint32 execFieldDialog(const QString& caption, QString& name, qint32& type, QString& defaultValue);
    qint32 execHeaderDialog(const QString& caption, QString& name);
    void validateControlsHeaders();
};

#endif // HEADERSVIEWCONTROL_H
