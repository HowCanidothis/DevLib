#ifndef DBIMPORTATTRIBUTES_H
#define DBIMPORTATTRIBUTES_H

#include <QString>

struct DbDescImportAttributes
{
    QString replace_fields;
    QString field_delegates;
    QString rename_fields;
    QString renames_tables;
    QString renames_heads;
    QString table_names;
    QString dont_import_fields;
    QString where_statements;
    QString select_delegates;
    QString additional_fields;
    QString merge_tables;
    QString merge_fields;

    bool is_import_tables = false;

    QString dbname;
    QString dbpassword;
    QString dbuser;
    QString dbdriver;
    qint32 dbport;
    QString dbhost;
    QString dbquery_get_tables;

    DbDescImportAttributes(){}
    DbDescImportAttributes(const QString& fName){ this->FromIniFile(fName); }

    void ToIniFile(const QString& fname) const;
    void FromIniFile(const QString& fname);
};


#endif
