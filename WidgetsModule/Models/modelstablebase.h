#ifndef MODELSTABLEBASE_H
#define MODELSTABLEBASE_H

#include <QAbstractTableModel>

#include "wrappers.h"

class ModelsTableBase : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
public:
    ModelsTableBase(QObject* parent);
    ~ModelsTableBase();

    void SetData(const ModelsTableWrapperPtr& data);
    const ModelsTableWrapperPtr& GetData() const { return m_data; }

protected:
    ModelsTableWrapperPtr m_data;
};

template<class T>
class TModelsTableBase : public ModelsTableBase
{
    using Super = ModelsTableBase;
public:
    using Super::Super;

//	Qt::ItemFlags flags(const QModelIndex& index = QModelIndex()) const override { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
//	int rowCount(const QModelIndex& parent = QModelIndex()) const override { return GetData()->GetSize(); }
//	bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
//	{
//		GetData()->Insert(row, count);
//		return true;
//	}
//	bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
//	{
//		QSet<qint32> indexs;
//		for(int i=0; i<count; ++i){
//			indexs.insert(row + i);
//		}
//		GetData()->Remove(indexs);
//		return true;
//	}
	
    void SetData(const SharedPointer<T>& data) { Super::SetData(data); }
    const SharedPointer<T>& GetData() const { return Super::GetData().template Cast<T>(); }
};

#endif // MODELSTABLEBASE_H
