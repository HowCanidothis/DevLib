#include "sqlitedatabase.h"

#include <QSqlQuery>
#include <QSqlError>

#include <SharedModule/Threads/ThreadFunction/thread.h>

SQLiteDatabase::SQLiteDatabase(const QString& connection, const QString& user, const QString& password)
    : Super(1)
    , m_deleted(::make_shared<std::atomic_bool>(false))
    , m_inTransaction(::make_shared<std::atomic_bool>(false))
{
    auto deleted = m_deleted;
    PushTask([this, connection, user, password, deleted]{
        if(*deleted) {
            return;
        }
        auto db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(connection);
        auto isValid = db.open(user, password);
        ThreadsBase::DoMain(CDL, [this, deleted, db, isValid]{
            if(*deleted) {
                return;
            }
            m_db = db;
            IsValid.SetState(isValid);
            OnInitialized.Resolve(true);
        });
    }, EPriority::High);
}

void SQLiteDatabase::Terminate()
{
    PushTask([this]{
        m_db.close();
    }, EPriority::Low);
    Await();
    *m_deleted = true;
    Super::Terminate();
}

AsyncResult SQLiteDatabase::QueryWhereIdIn(const QString& operation, const QSet<Name>& ids, Op op)
{
    return Transaction([operation, op, ids](QSqlDatabase& db){
        QSqlQuery query(db);
        if(ids.isEmpty()) {
            return query.exec(operation);
        }

        if(ids.size() < 1000) {
            StringBuilder placeholders;
            for (int i = 0; i < ids.size(); ++i) {
                placeholders.Add(",", "?");
            }

            // Secure: The structure is prepared with placeholders
            query.prepare(QString("%1 WHERE id %2IN (%3)")
                          .arg(operation, op == In ? "" : "NOT ", placeholders));

            // Secure: Values are bound safely by the driver
            for (const auto& id : ids) {
                query.addBindValue(id.AsString());
            }

            return query.exec();
        }

        QVariantList vl;
        for(const auto& id : ids) {
            vl.append(id.AsString());
        }
        // 1. Create a temporary table (lives only for this session)
        query.exec("CREATE TEMPORARY TABLE IF NOT EXISTS ids_to_keep (id TEXT PRIMARY KEY)");
        // 2. Batch insert your IDs into the temp table
        query.prepare("INSERT OR REPLACE INTO ids_to_keep (id) VALUES (?)");
        query.addBindValue(vl);
        if (!query.execBatch()) return false;

         QString sql = QString(
            "%1 WHERE id %2IN (SELECT id FROM ids_to_keep)"
        ).arg(operation, op == In ? QString() : "NOT ");
        if (!query.exec(sql)) return false;

        // 4. Cleanup temp table (optional, but good practice)
        return query.exec("DROP TABLE ids_to_keep");
    });
}

AsyncResult SQLiteDatabase::IdExists(const QString& tableName, const Name& id)
{
    return Query([tableName, id](QSqlQuery& query) {
        if(!query.prepare(QString("SELECT EXISTS(SELECT 1 FROM %1 WHERE id = :id)").arg(tableName))) {
            return false;
        }
        query.bindValue(":id", id.AsString());
        if (query.exec() && query.next()) {
            return query.value(0).toBool();
        }
        return false;
    });
}

AsyncResult SQLiteDatabase::Transaction(const std::function<bool (QSqlDatabase&)>& transaction)
{
    auto db = m_db;
    auto inTransaction = m_inTransaction;
    auto h = [db, transaction, inTransaction]{
        auto& ncdb = const_cast<QSqlDatabase&>(db);
        if(!ncdb.transaction()) {
            return false;
        }
        guards::LambdaGuard g([inTransaction]{
            *inTransaction = false;
        }, [inTransaction]{
            *inTransaction = true;
        });
        if(!transaction(ncdb)) {
            ncdb.rollback();
            return false;
        }
        ncdb.commit();
        return true;
    };
    if(m_threads.First() == QThread::currentThread()) {
        if(*m_inTransaction) {
            if(transaction(db)) {
                return AsyncSuccess();
            }
            return AsyncError();
        }
        if(h()) {
            return AsyncSuccess();
        }
        return AsyncError();
    }
    return PushTask([h]{
        if(!h()) {
            throw 0;
        }
    }, EPriority::High);
}

AsyncResult SQLiteDatabase::Query(const std::function<bool (QSqlQuery& query)>& handler)
{
    auto db = m_db;
    auto h = [db, handler]{
        QSqlQuery query(db);
        return handler(query);
    };

    if(m_threads.First() == QThread::currentThread()) {
        if(h()) {
            return AsyncSuccess();
        }
        return AsyncError();
    }
    return PushTask([h]{
        if(!h()) {
            throw 0;
        }
    }, EPriority::High);
}

SP<SQLiteDatabase> SQLiteDatabase::Create(const QString& connection, const QString& user, const QString& password)
{
    return SP<SQLiteDatabase>(new SQLiteDatabase(connection, user, password));
}

SQLiteDatabase::~SQLiteDatabase()
{
}

