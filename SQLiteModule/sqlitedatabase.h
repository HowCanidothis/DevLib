#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>

#include <PropertiesModule/internal.hpp>

class SQLiteDatabase : public ThreadPool
{
    using Super = ThreadPool;

    SQLiteDatabase(const QString& connection, const QString& user, const QString& password);
public:
    virtual ~SQLiteDatabase();

    static SP<SQLiteDatabase> Create(const QString& connection, const QString& user, const QString& password);
    AsyncResult Transaction(const std::function<bool (QSqlDatabase&)>& transaction);
    AsyncResult Query(const std::function<bool (QSqlQuery&)>& handler);
    AsyncResult IdExists(const QString& tableName, const Name& id);

    enum Op
    {
        In,
        NotIn
    };

    AsyncResult QueryWhereIdIn(const QString& operation, const QSet<Name>& ids, Op = In);

    StateProperty IsValid;
    AsyncResult OnInitialized;

private:
    QSqlDatabase m_db;
    SP<std::atomic_bool> m_deleted;
    SP<std::atomic_bool> m_inTransaction;
};

enum class SQLiteDatabaseFieldType
{
    Undefined,
    Text,
    Int,
    Blob
};

template<class T>
struct SQLiteTypeFor
{
//    static constexpr SQLiteDatabaseFieldType Type = SQLiteDatabaseFieldType::Undefined;
};

template<>
struct SQLiteTypeFor<Name>
{
    static constexpr SQLiteDatabaseFieldType Type = SQLiteDatabaseFieldType::Text;
    static QVariant ToVariant(const Name& n){ return QVariant::fromValue(n.AsString()); };
    static void FromVariant(const QVariant& v, Name& n){ n = Name(v.toString()); };
};

template<>
struct SQLiteTypeFor<qint32>
{
    static constexpr SQLiteDatabaseFieldType Type = SQLiteDatabaseFieldType::Int;
    static QVariant ToVariant(qint32 n){ return QVariant::fromValue(n); };
    static void FromVariant(const QVariant& v, qint32& n){ n = v.toDouble(); };
};

template<>
struct SQLiteTypeFor<QByteArray>
{
    static constexpr SQLiteDatabaseFieldType Type = SQLiteDatabaseFieldType::Blob;
    static QVariant ToVariant(const QByteArray& n){ return QVariant::fromValue(n); };
    static void FromVariant(const QVariant& v, QByteArray& n){ n = v.toByteArray(); };
};

template<>
struct SQLiteTypeFor<QDateTime>
{
    static constexpr SQLiteDatabaseFieldType Type = SQLiteDatabaseFieldType::Int;
    static QVariant ToVariant(const QDateTime& n){ return QVariant::fromValue(n.currentSecsSinceEpoch()); };
    static void FromVariant(const QVariant& v, QDateTime& n){ n = QDateTime::fromSecsSinceEpoch(v.toDouble(), Qt::UTC); };
};

template<>
struct SQLiteTypeFor<QString>
{
    static constexpr SQLiteDatabaseFieldType Type = SQLiteDatabaseFieldType::Text;
    static QVariant ToVariant(const QString& n){ return QVariant::fromValue(n); };
    static void FromVariant(const QVariant& v, QString& n){ n = v.toString(); };
};

template<>
struct SQLiteTypeFor<bool>
{
    static constexpr SQLiteDatabaseFieldType Type = SQLiteDatabaseFieldType::Int;
    static QVariant ToVariant(bool n){ return QVariant::fromValue(n); };
    static void FromVariant(const QVariant& v, bool& n){ n = v.toBool(); };
};

template<class T>
struct SQLiteMappingValue
{
    using FToVariant = std::function<QVariant (const T&)>;
    using FFromVariant = std::function<void (T&, const QVariant&)>;

    SQLiteMappingValue()
        : Type(SQLiteDatabaseFieldType::Undefined)
        , IsPrimary(false)
    {}

    template<typename V>
    static SQLiteMappingValue<T> Create(const std::function<V& (T&)>& fieldGetter, bool primary = false) {
        SQLiteMappingValue<T> result;
        result.ToVariant = [fieldGetter](const T& s){ return SQLiteTypeFor<V>::ToVariant(fieldGetter(const_cast<T&>(s))); };
        result.FromVariant = [fieldGetter](T& s, const QVariant& v){ SQLiteTypeFor<V>::FromVariant(v,fieldGetter(s)); };
        result.Type = SQLiteTypeFor<V>::Type;
        return result;
    }

    static SQLiteMappingValue<T> CreateId(const std::function<Name& (T&)>& fieldGetter) {
        SQLiteMappingValue<T> result;
        result.ToVariant = [fieldGetter](const T& s){ return SQLiteTypeFor<Name>::ToVariant(fieldGetter(const_cast<T&>(s))); };
        result.FromVariant = [fieldGetter](T& s, const QVariant& v){ SQLiteTypeFor<Name>::FromVariant(v,fieldGetter(s)); };
        result.Type = SQLiteTypeFor<Name>::Type;
        result.IsPrimary = true;
        return result;
    }

    static SQLiteMappingValue<T> CreateCreatedUpdatedAt(const std::function<QDateTime& (T&)>& fieldGetter) {
        SQLiteMappingValue<T> result;
        result.ToVariant = [fieldGetter](const T& s){ return SQLiteTypeFor<QDateTime>::ToVariant(fieldGetter(const_cast<T&>(s))); };
        result.FromVariant = [fieldGetter](T& s, const QVariant& v){ SQLiteTypeFor<QDateTime>::FromVariant(v,fieldGetter(s)); };
        result.Type = SQLiteTypeFor<QDateTime>::Type;
        result.Default = " DEFAULT (CAST(strftime('%s', 'now') AS INTEGER))";
        return result;
    }


    FToVariant ToVariant;
    FFromVariant FromVariant;
    SQLiteDatabaseFieldType Type;
    bool IsPrimary;
    QString Key;
    QString Default;

    QString Field() const
    {
        StringBuilder result;
        switch(Type) {
        case SQLiteDatabaseFieldType::Int: result.Add(QString(), "INT"); break;
        case SQLiteDatabaseFieldType::Text: result.Add(QString(), "TEXT"); break;
        }
        if(IsPrimary) {
            result.Add(" ", "PRIMARY KEY");
        }
        if(!Default.isEmpty()) {
            result.Add(" ", Default);
        }
        return std::move(result);
    }
};

template<class T>
class SQLiteMapping : public QMap<QString, SQLiteMappingValue<T>>
{
    using Super = QMap<QString, SQLiteMappingValue<T>>;
public:
    using Super::Super;

    QString Fields() const
    {
        StringBuilder result;
        for(auto it(cbegin()), e(cend()); it != e; ++it) {
            result.Add(", ", it.key() + " " + it.value().Field());
        }
        return std::move(result);
    }

    static QString FieldNames(const QVector<SQLiteMappingValue<T>>& mappers)
    {
        return StringBuilder().Join(",", mappers, [](const auto& it){ return it->Key; });
    }

    static QString FieldValues(const QVector<SQLiteMappingValue<T>>& mappers)
    {
        return StringBuilder().Join(",", mappers, [](const auto it){ return ":" + it->Key; });
    }
};

struct SQLiteModelsTableModelParams
{
    bool AddUpdatedAtTimeStamp = false;
    bool AddDeletedFlag = false;
    SQLiteModelsTableModelParams& AddUpdatedAt() { AddUpdatedAtTimeStamp = true; return *this; }
    SQLiteModelsTableModelParams& AddMarkedAsDeleted() { AddDeletedFlag = true; return *this; }

};

template<class T>
class TSQLiteModelsTableModel : public TModelsTableWrapper<QVector<T>>
{
    using Super = TModelsTableWrapper<QVector<T>>;
public:
    using FIdGetter = std::function<const Name& (const T&)>;

    TSQLiteModelsTableModel(const SP<SQLiteDatabase>& database, const QString& tableName, const FIdGetter& idGetter, SQLiteModelsTableModelParams params = SQLiteModelsTableModelParams())
        : m_db(database)
        , m_tableName(tableName)
        , m_deleted(::make_shared<std::atomic_bool>(false))
        , m_idGetter(idGetter)
        , m_markAsDeleted(params.AddDeletedFlag)
    {
        if(database->IsValid) {
            //Test table existance
            database->Query([tableName](QSqlQuery& query) {
                return query.exec(QString("CREATE TABLE IF NOT EXISTS %1 (%2)").arg(tableName, T::Mapping.Fields()));
            });
            if(params.AddUpdatedAtTimeStamp) {
                database->Query([tableName](QSqlQuery& query) {
                    return query.exec(QString("CREATE TRIGGER IF NOT EXISTS update_timestamp AFTER UPDATE ON %1 \
                                                BEGIN \
                                                    UPDATE %1 SET updated_at = CAST(strftime('%s', 'now') AS INTEGER) WHERE id = OLD.id; \
                                                END;").arg(tableName));
                });
            }
        } else {
            qCCritical(LC_UI) << QObject::tr("Unable to connect to database");
        }
    }

    ~TSQLiteModelsTableModel()
    {
        *m_deleted = true;
    }

    void Connect(const QString& what)
    {
        Disconnect();
        connectCRUD(generateMappers(what), what);
    }

    void Disconnect()
    {
        m_selectConnections.clear();
        m_currentIds.clear();
        Super::Clear();
    }

    AsyncRequest Select(const QString& what, const QString& where, const std::function<void (const QVector<T>&)>& onResult)
    {
        AsyncRequest result;
        auto tname = m_tableName;
        auto r = m_db->Query([tname,what,where,onResult,result](QSqlQuery& query){
            if(result.ResultInterruptor.IsInterrupted()) {
                result.Result.Resolve(false);
                return true;
            }
            QVector<T> resultValues;
            if(!select(generateMappers(what), what, where, tname, query, resultValues)) {
                result.Result.Resolve(false);
                return false;
            }
            ThreadsBase::DoMainWithResult(CDL, [resultValues, onResult, result]{
                if(result.ResultInterruptor.IsInterrupted()) {
                    result.Result.Resolve(false);
                    return;
                }
                onResult(resultValues);
                result.Result.Resolve(true);
            });
            return true;
        });
        return result;
    }

    void Select(const QString& what, const QString& where = QString())
    {
        QVector<SQLiteMappingValue<T>> mappers = generateMappers(what);
        Disconnect();
        const auto& idGetter = m_idGetter;
        auto tname = m_tableName;
        auto deleted = m_deleted;
        m_db->Query([mappers, this, idGetter, what, where, tname, deleted](QSqlQuery& query){
            if(*deleted) {
                return true;
            }
            QVector<T> resultValues;
            if(!select(mappers, what, where, tname, query, resultValues)) {
                return false;
            }

            ThreadsBase::DoMain(CDL, [this, mappers, what, idGetter, deleted, resultValues]{
                if(*deleted) {
                    return;
                }
                Super::Change([&](QVector<T>& r){
                    for(const auto& v : resultValues) {
                        r.append(v);
                        m_currentIds.insert(idGetter(v));
                    }
                });
                connectCRUD(mappers, what);
            });
            return true;
        });
    }
private:
    AsyncResult deleteWhere(const QString& tableName, const QSet<Name>& ids)
    {
        if(m_markAsDeleted) {
            return m_db->QueryWhereIdIn(QString("UPDATE %1 SET marked_as_deleted = 1").arg(tableName), ids);
        } else {
            return m_db->QueryWhereIdIn(QString("DELETE FROM %1").arg(tableName), ids);
        }
    }

    void connectCRUD(const QVector<SQLiteMappingValue<T>>& mappers, const QString& what)
    {
        auto idGetter = m_idGetter;
        auto insertOrUpdate([this,mappers](const QVector<QVariantList>& values) {
            if(!values[0].isEmpty()) {
                auto tableName = m_tableName;
                m_db->Transaction([mappers, tableName, values](QSqlDatabase& db) {
                    QSqlQuery query(db);
                    auto fieldNames = T::Mapping.FieldNames(mappers);
                    auto fieldValues = T::Mapping.FieldValues(mappers);
                    if(!query.prepare(QString("INSERT OR REPLACE INTO %1 (%2) VALUES (%3)").arg(tableName, fieldNames, fieldValues))) {
                        return false;
                    }

                    for (const QVariantList& columnData : values) {
                        query.addBindValue(columnData);
                    }
                    return query.execBatch();
                });
            }
        });

        auto update([this, what, mappers](const QVector<QVariantList>& values, const QVariantList& ids) {
            if(!values[0].isEmpty()) {
                auto tableName = m_tableName;
                m_db->Transaction([mappers, tableName, values, ids, what](QSqlDatabase& db) {
                    QSqlQuery query(db);
                    QStringList setClauses;
                    for (const auto& mapper : mappers) {
                        if(mapper.IsPrimary) {
                            continue;
                        }
                        setClauses << QString("%1 = ?").arg(mapper.Key);
                    }

                    if(!query.prepare(QString("UPDATE %1 SET %2 WHERE id = ?").arg(tableName, setClauses.join(", ")))) {
                        return false;
                    }

                    for (const QVariantList& columnData : values) {
                        query.addBindValue(columnData);
                    }
                    query.addBindValue(ids);
                    return query.execBatch();
                });
            }
        });

        Super::OnRowsChanged.Connect(CDL, [this, update, idGetter, mappers](qint32 row, qint32 count, const QSet<qint32>&) {
            QVector<QVariantList> values(mappers.size());
            QVariantList ids;
            for(const auto& v : adapters::range(begin() + row, begin() + row + count)) {
                if(idGetter(v).IsNull()) {
                    continue;
                }
                adapters::Foreach([&](const SQLiteMappingValue<T>& map, QVariantList& l) {
                    if(map.IsPrimary) {
                        ids.append(map.ToVariant(v));
                    } else {
                        l.append(map.ToVariant(v));
                    }
                }, mappers, values);
            }
            QVector<QVariantList> valuesWithoutEmpty;
            for(const auto& v : values) {
                if(!v.isEmpty()) {
                    valuesWithoutEmpty.append(v);
                }
            }
            update(valuesWithoutEmpty, ids);
        }).MakeSafe(m_selectConnections);

        Super::OnChanged.Connect(CDL, [this, insertOrUpdate, idGetter, mappers]{
            QVector<QVariantList> values(mappers.size());
            QSet<Name> cids;
            for(const auto& v : this->Native()) {
                if(idGetter(v).IsNull()) {
                    continue;
                }
                const auto& id = idGetter(v);
                if(!m_currentIds.contains(id)) {
                    adapters::Foreach([&](const SQLiteMappingValue<T>& map, QVariantList& l) {
                        l.append(map.ToVariant(v));
                    }, mappers, values);
                }
                cids.insert(id);
            }
            auto removed = m_currentIds - cids;
            auto tableName = m_tableName;
            m_currentIds = cids;

            if(!removed.isEmpty()) {
                deleteWhere(tableName, removed);
            }
            insertOrUpdate(values);
        }).MakeSafe(m_selectConnections);
    }

    static QVector<SQLiteMappingValue<T>> generateMappers(const QString& what)
    {
        QVector<SQLiteMappingValue<T>> mappers;
        if(what == "*") {
            auto keys = T::Mapping.keys();
            auto values = T::Mapping.values();
            adapters::Foreach([&](const QString& key, const SQLiteMappingValue<T>& imapper){
                auto mapper = imapper;
                mapper.Key = key;
                mappers.append(mapper);
            }, keys, values);
        } else {
            for(const auto& field : what.split(",")) {
                auto foundIt = T::Mapping.constFind(field);
                if(foundIt == T::Mapping.cend()) {
                    Q_ASSERT(false);
                } else {
                    auto mapper = foundIt.value();
                    mapper.Key = foundIt.key();
                    mappers.append(mapper);
                }
            }
        }
        return mappers;
    }

    static bool select(const QVector<SQLiteMappingValue<T>>& mappers, const QString& what, const QString& where, const QString& tname, QSqlQuery& query, QVector<T>& resultValues)
    {
        auto result = query.exec(QString("SELECT %1 FROM %2 %3").arg(what, tname, where));
        if(!result) {
            return false;
        }

        while (query.next()) {
            T value;

            qint32 i(0);
            for(const auto& m : mappers) {
                m.FromVariant(value, query.value(i));
                ++i;
            }

            resultValues.append(value);
        }
        if(resultValues.isEmpty()) {
            return true;
        }
        return true;
    }

private:
    SP<SQLiteDatabase> m_db;
    QString m_tableName;
    SP<std::atomic_bool> m_deleted;
    QSet<Name> m_currentIds;
    DispatcherConnectionsSafe m_selectConnections;
    FIdGetter m_idGetter;
    bool m_markAsDeleted;
};

#endif // SQLITEDATABASE_H
