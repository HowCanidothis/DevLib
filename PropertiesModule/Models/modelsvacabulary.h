#ifndef MODELSVACABULARY_H
#define MODELSVACABULARY_H

template<class T>
class TViewModelsListBase;

using ModelsVocabularyRequest = CommonDispatcher<qint32>;

class ModelsVocabulary : public TModelsTableWrapper<QVector<QHash<Name, QVariant>>>
{
    using Super = TModelsTableWrapper<QVector<QHash<Name, QVariant>>>;
public:
    struct HeaderDataValue
    {
        Name ColumnKey;
        TranslatedStringPtr Label;
        FMeasurementGetter Measurement;
    };
    using HeaderData = QVector<HeaderDataValue>;

    template<class T>
    static QVector<SharedPointer<ModelsVocabulary>> CreateFromStruct(const T& headerValues, qint32 count)
    {
        Q_ASSERT(count > 1);
        QVector<SharedPointer<ModelsVocabulary>> result;
        HeaderData header;
        adapters::ForeachFieldOfStruct<HeaderDataValue>(headerValues, [&header](const HeaderDataValue& value){
            header.append(value);
        });
        while(count--) {
            result.append(::make_shared<ModelsVocabulary>(header));
        }
        return result;
    }
    template<class T>
    static SharedPointer<ModelsVocabulary> CreateFromStruct(const T& headerValues)
    {
        HeaderData result;
        adapters::ForeachFieldOfStruct<HeaderDataValue>(headerValues, [&result](const HeaderDataValue& value){
            result.append(value);
        });
        return ::make_shared<ModelsVocabulary>(result);
    }
    ModelsVocabulary(const HeaderData& dictionary);

    template<class Property>
    DispatcherConnection CreatePropertyConnection(LocalPropertyOptional<Property>* property, const Name& name, ModelsVocabularyRequest* indexDispatcher)
    {
        return indexDispatcher->Connect(CONNECTION_DEBUG_LOCATION, [this, name, property](qint32 index){
            if(!IsValidRow(index)) {
                return;
            }
            property->FromVariant(SelectValue(name, At(index)));
        });
    }

    template<class Property>
    DispatcherConnection CreatePropertyConnection(Property* property, const Name& name, ModelsVocabularyRequest* indexDispatcher)
    {
        return indexDispatcher->Connect(CONNECTION_DEBUG_LOCATION, [this, name, property](qint32 index){
            if(!IsValidRow(index)) {
                return;
            }
            typename Property::FValidator validator = [](const typename Property::value_type& value) { return value; };
            LocalPropertySetFromVariant<Property>(*property, SelectValue(name, At(index)), validator);
        });
    }

    static const QVariant& SelectValue(const Name& name, const QHash<Name, QVariant>& row);

    qint32 GetColumnsCount() const { return m_header.size(); }
    const HeaderDataValue& GetHeader(qint32 column) const;
    const QVector<HeaderDataValue>& GetHeader() const { return m_header; }
#ifdef WIDGETS_MODULE_LIB
    static TViewModelsListBase<ModelsVocabulary>* CreateListModel(qint32 column, QObject* parent);
#endif

private:
    QVector<HeaderDataValue> m_header;
    DispatcherConnectionsSafe m_connections;
};
using ModelsVocabularyPtr = SharedPointer<ModelsVocabulary>;

#endif // MODELSVACABULARY_H
