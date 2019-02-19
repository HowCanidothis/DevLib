#ifndef DBIMPORTPARSER_H
#define DBIMPROTPARSER_H

class QSqlRecord;
class Database;

struct DatabaseImportProperties
{
    struct Renames{
        String table;
        String head;
    };

    typedef ModifiedFieldSimple FieldAssociation;
    typedef String TableKey;
    typedef String TableFieldKey;

    typedef Hash<TableKey,Vector<FieldAssociation>> FieldsReplaceMap; //key is table
    typedef FieldsReplaceMap::const_iterator FieldsReplaceMapCIt;
    typedef QMap<TableFieldKey, FieldAssociation> TableAssociation; //key is table field
    typedef TableAssociation::const_iterator TableAssociationCIt;
    typedef Set<TableFieldKey> FieldsDontImport; //key is table field
    typedef Hash<TableKey,TableAssociation> FieldsTypeMap; //key is table
    typedef FieldsTypeMap FieldMergeMap;
    typedef FieldMergeMap::const_iterator FieldMergeMapCIt;
    typedef Hash<TableKey, Renames> RenamesMap; //key is table
    typedef Hash<TableKey, FieldsDontImport> DontImportFieldsMap; //key is table
    typedef SortedVector<TableKey> DontImportTablesMap; //value is table
    typedef Hash<TableKey,String> SelectStatementsMap;
    typedef SelectStatementsMap::const_iterator SelectStatementsMapCIt;
    typedef Hash<TableFieldKey, String> SelectTableAssociation;
    typedef SelectTableAssociation::const_iterator SelectTableAssociationCIt;
    typedef Hash<TableKey, SelectTableAssociation> SelectDelegatesMap;
    typedef SelectDelegatesMap::const_iterator SelectDelegatesMapCit;

    //In cpp export
    void setPath(const String& path){ dbhost = path; }
    void setNamespace(const String pref){ dbuser = pref; }
    String getPath() const { return dbhost; }
    String getNamespace() const { return dbuser; }

    FieldMergeMap generateFieldsMergeMap() const;
    SelectDelegatesMap generateSelectDelegatesMap() const;
    SelectStatementsMap generateSelectWhereMap() const;
    FieldsTypeMap generateFieldsTypeMap() const;
    RenamesMap generateRenamesMap() const;
    DontImportTablesMap generateTablesImportMap() const;
    DontImportFieldsMap generateFieldsDontImportMap() const;
    FieldsReplaceMap generateFieldsReplaceMap() const;
    FieldsReplaceMap generateFieldsAdditionalMap() const;

    String simplify(const String &src) const;

    static DatabaseImportProperties & fromBase( DatabaseImportPropertiesBase & base ){ return *((DatabaseImportProperties*)&base);  }
    static const DatabaseImportProperties & fromBase(const DatabaseImportPropertiesBase & base ){ return *((DatabaseImportProperties*)&base);  }

    DatabaseImportProperties(){}
    DatabaseImportProperties(const String& fName) : DatabaseImportPropertiesBase(fName){}

private:
    FieldsTypeMap pGenerateTypeMap(const String& string) const;
};

inline String DatabaseImportProperties::simplify(const String &src) const
{
    QString res = src;
    QRegExp re("\\n");
    return res.remove(re);
}

struct Field{
    qint32 type;
    String name;
    Field(qint32 t, const String & name);

    Field(QVariant::Type intype, const String& inname, const DatabaseImportProperties::TableAssociation &map, const DatabaseImportProperties::TableAssociation & global);

    bool operator==(const Field& other) const { return this->type == other.type && this->name == other.name; }
};

static inline uint qHash(const Field& rec, uint seed=0){
    return qHash(rec.name,seed);
}


class SimpleHead : public Vector<Field>
{
public:
    struct InitProperties{
        Vector<DatabaseImportProperties::FieldAssociation> additional_fields;
        DatabaseImportProperties::TableAssociation types_map;
        DatabaseImportProperties::TableAssociation global_types_map;
        DatabaseImportProperties::FieldsDontImport dont_import_map;
        DatabaseImportProperties::FieldsDontImport global_dont_import_map;
        InitProperties(const Vector<DatabaseImportProperties::FieldAssociation>& af, const DatabaseImportProperties::TableAssociation& t, const DatabaseImportProperties::TableAssociation& gt,
                   const DatabaseImportProperties::FieldsDontImport& rf, const DatabaseImportProperties::FieldsDontImport& grf)
            : additional_fields(af), types_map(t), global_types_map(gt), dont_import_map(rf), global_dont_import_map(grf){}

        bool isImportableField(const String& lower_fname) const { return !(dont_import_map.contains(lower_fname) || global_dont_import_map.contains(lower_fname)); }
    };

    static SimpleHead from(const QSqlRecord& rec,const InitProperties &props);

    bool operator ==(const SimpleHead& other) const{
        if(this->size() != other.size()) return false;
        SimpleHead::const_iterator b = other.begin();
        for(const Field& f : *this){
            if(!(f == *b++)) return false;
        }
        return true;
    }
};

inline static uint qHash(const SimpleHead& rec, uint seed=0){
    return qHashRangeCommutative(rec.begin(), rec.end(), seed);
}




#endif // IMPORTPROPERTIES_H
