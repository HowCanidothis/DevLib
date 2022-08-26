#ifndef PROPERTIESTABLEMODEL_H
#define PROPERTIESTABLEMODEL_H

#include <QAbstractTableModel>

#include <PropertiesModule/internal.hpp>

class PropertiesTableModel : public QAbstractTableModel
{
    using Super = QAbstractTableModel;

    struct Row
    {
        QHash<Name, Property*> Properties;
    };

public:
    PropertiesTableModel(QObject* parent = nullptr);

    void Init(const PropertiesScopeName& scope, const FAction& initializationFunc, const FAction& onEveryChange = FAction());
    void Update();

public:
    LocalProperty<PropertiesScopeName> ContextIndex;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

private:
    Property* asProperty(const QModelIndex& index) const;

private:
    QHash<Name, Row> m_rows;
    QVector<Name> m_headers;
    FAction m_initializationFunction;
    FAction m_onEveryChange;    
};

#endif // PROPERTIESTABLEMODEL_H
