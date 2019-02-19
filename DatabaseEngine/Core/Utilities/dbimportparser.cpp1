#include "databaseimportproperties.h"
#include "tablefield.h"
#include <QSqlRecord>
#include <QSqlField>

Field::Field(qint32 t, const String &name) : type(t), name(name) {}

Field::Field(QVariant::Type intype, const String &inname, const DatabaseImportProperties::TableAssociation &map, const DatabaseImportProperties::TableAssociation &global){
    this->name = inname;
    DatabaseImportProperties::TableAssociation::const_iterator found = map.find(name);
    if(found != map.end()){
        DatabaseImportProperties::FieldAssociation fas = found.value();
        if(fas.type != FieldType::NotStorable){
            type = fas.type;
            return;
        }
    }
    else{
        found = global.find(name);
        if(found != global.end()){
            DatabaseImportProperties::FieldAssociation fas = found.value();
            if(fas.type != FieldType::NotStorable){
                type = fas.type;
                return;
            }
        }
    }
    switch ((qint32)intype) {
    case QVariant::Int: type = FieldType::Long; break;
    case QVariant::Bool: type = FieldType::Boolean; break;
    case QVariant::Double: type = FieldType::Float; break;
    case QVariant::String: type = FieldType::Text; break;
    case QVariant::DateTime: type = FieldType::DateTime; break;
    case QVariant::ByteArray: type = FieldType::Blob; break;
    case QVariant::UInt: type = FieldType::UnsignedLong; break;
    case QVariant::LongLong: type = FieldType::LongLong; break;
    case QVariant::ULongLong: type = FieldType::UnsignedLongLong; break;
    case QMetaType::UChar: type = FieldType::UnsignedByte; break;
    case QMetaType::Short: type = FieldType::Short; break;
    case QMetaType::UShort: type = FieldType::UnsignedShort; break;
    case QMetaType::Char: type = FieldType::Byte; break;

    default:
        type = FieldType::Text;
        qDebug() << "undeclared database type" << intype << "converted to text";
        break;
    }
}

SimpleHead SimpleHead::from(const QSqlRecord &rec, const SimpleHead::InitProperties &props){
    SimpleHead res;
    for(qint32 i(0); i < rec.count(); i++)
    {
        QSqlField field = rec.field(i);
        String lower_name = field.name();
        if(lower_name.toLower() != "id" && props.isImportableField(lower_name))
            res.append(Field(field.type(), lower_name, props.types_map, props.global_types_map));
    }
    for(const DatabaseImportProperties::FieldAssociation& fas : props.additional_fields)
        res.append(Field(fas.type,fas.new_name));
    return res;
}


DatabaseImportProperties::FieldMergeMap DatabaseImportProperties::generateFieldsMergeMap() const
{
    return pGenerateTypeMap(this->merge_fields);
}

DatabaseImportProperties::SelectDelegatesMap DatabaseImportProperties::generateSelectDelegatesMap() const
{
    QRegExp re("\\[([^\\]]*)\\]([^-]*)-([^;]*)");
    SelectDelegatesMap res;
    qint32 pos(0);
    while ((pos = re.indexIn(simplify(this->select_delegates),pos)) != -1) {
        String field = re.cap(2);
        String delegate = re.cap(3);
        for(const String& tname :  re.cap(1).split(",", String::SkipEmptyParts))
            res[tname][field] = delegate;
        pos += re.matchedLength();
    }
    return res;
}

DatabaseImportProperties::SelectStatementsMap DatabaseImportProperties::generateSelectWhereMap() const
{
    QRegExp re("\\[([^\\]]*)\\]([^;]*)");
    SelectStatementsMap res;
    qint32 pos(0);
    while ((pos = re.indexIn(simplify(this->where_statements),pos)) != -1) {
        String select = re.cap(2);
        for(const String& tname :  re.cap(1).split(",", String::SkipEmptyParts))
            res[tname] = select;
        pos += re.matchedLength();
    }
    return res;
}

DatabaseImportProperties::FieldsTypeMap DatabaseImportProperties::generateFieldsTypeMap() const
{
    FieldsTypeMap res = pGenerateTypeMap(this->field_delegates);

    QRegExp re = QRegExp("\\[([^\\]]*)\\]([^;]*)");
    QRegExp red("([^-]*)-(.*)");

    qint32 pos=0;
    while ((pos = re.indexIn(simplify(this->rename_fields),pos)) != -1) {
        for(const String& table_name :  re.cap(1).split(",", String::SkipEmptyParts)){
            String fields = re.cap(2);
            for(const String f : fields.split(",", String::SkipEmptyParts)){
                red.indexIn(f);
                String fname = red.cap(1);
                String nfname = red.cap(2);
                TableAssociation & ta = res[table_name];
                FieldAssociation & fa = ta[fname];
                fa.old_name = fname;
                fa.new_name = nfname;
            }
        }
        pos += re.matchedLength();
    }

    return res;
}

DatabaseImportProperties::RenamesMap DatabaseImportProperties::generateRenamesMap() const
{
    QRegExp re("\\[([^\\]]*)\\]([^;]*)");
    RenamesMap res;
    qint32 pos(0);
    while ((pos = re.indexIn(simplify(this->renames_tables),pos)) != -1) {
        String name_after = re.cap(2);
        for(const String& name :  re.cap(1).split(",", String::SkipEmptyParts))
            res[name].table = name_after;
        pos += re.matchedLength();
    }

    pos=0;
    while ((pos = re.indexIn(simplify(this->renames_heads),pos)) != -1) {
        String name_after = re.cap(2);
        for(const String& name :  re.cap(1).split(",", String::SkipEmptyParts))
            res[name].head = name_after;
        pos += re.matchedLength();
    }

    return res;
}

DatabaseImportProperties::DontImportTablesMap DatabaseImportProperties::generateTablesImportMap() const
{
    DontImportTablesMap res;
    for(const String& n : simplify(this->table_names).split(";",QString::SkipEmptyParts))
        res.insertUnique(n.simplified());
    return res;
}

DatabaseImportProperties::DontImportFieldsMap DatabaseImportProperties::generateFieldsDontImportMap() const
{
    QRegExp re("\\[([^\\]]*)\\]([^;]*)");
    qint32 pos(0);
    DontImportFieldsMap res;
    while ((pos = re.indexIn(simplify(this->dont_import_fields),pos)) != -1) {
        String fields = re.cap(2);
        for(const String& table_name :  re.cap(1).split(",", String::SkipEmptyParts)){
            FieldsDontImport & ta = res[table_name];
            for(const String& s : fields.split(",", String::SkipEmptyParts))
                ta.insert(s);
        }
        pos += re.matchedLength();
    }
    return res;
}

DatabaseImportProperties::FieldsReplaceMap DatabaseImportProperties::generateFieldsReplaceMap() const
{
    QRegExp re("\\[([^\\]]*)\\]([^;]*)");
    QRegExp red("([^\\(-]*)\\(?([^\\)-;]*)\\)?-?(.*)");
    red.setPatternSyntax(QRegExp::RegExp2);
    qint32 pos(0);
    FieldsReplaceMap res;
    while ((pos = re.indexIn(simplify(this->replace_fields),pos)) != -1) {
        String fields = re.cap(2);
        for(const String& table_name :  re.cap(1).split(",", String::SkipEmptyParts)){
            Vector<FieldAssociation> & ta = res[table_name];
            for(const String& s : fields.split(",", String::SkipEmptyParts)){
                FieldAssociation fa;
                red.indexIn(s);
                fa.new_name = red.cap(1);
                fa.old_name = red.cap(2);
                fa.old_name = fa.old_name.isEmpty() ? fa.new_name : fa.old_name;
                fa.type = TableField::typeFromText(red.cap(3));
                ta.append(fa);
            }
        }
        pos += re.matchedLength();
    }
    return res;
}

DatabaseImportProperties::FieldsReplaceMap DatabaseImportProperties::generateFieldsAdditionalMap() const
{
    QRegExp re("\\[([^\\]]*)\\]([^;]*)");
    QRegExp red("([^-]*)-(.*)");

    qint32 pos(0);
    FieldsReplaceMap res;
    while ((pos = re.indexIn(simplify(this->additional_fields),pos)) != -1) {
        String fields = re.cap(2);
        for(const String& table_name :  re.cap(1).split(",", String::SkipEmptyParts)){
            Vector<FieldAssociation> & ta = res[table_name];
            for(const String& s : fields.split(",", String::SkipEmptyParts)){
                red.indexIn(s);
                FieldAssociation fa;
                fa.new_name = fa.old_name = red.cap(1);
                fa.type = TableField::typeFromText(red.cap(2));
                ta.append(fa);
            }
        }
        pos += re.matchedLength();
    }
    return res;
}

DatabaseImportProperties::FieldsTypeMap DatabaseImportProperties::pGenerateTypeMap(const String &string) const
{
    QRegExp re("\\[([^\\]]*)\\]([^;]*)");
    QRegExp re_sub("([^-]*)-([^;^,]*)");
    FieldsTypeMap res;
    qint32 pos(0);
    while ((pos = re.indexIn(simplify(string),pos)) != -1) {
        String fields_delegates = re.cap(2);
        TableAssociation table_association;
        qint32 pos_sub=0;
        while((pos_sub = re_sub.indexIn(fields_delegates,pos_sub)) != -1){
            QString fields = re_sub.cap(1);
            QString type_str = re_sub.cap(2);

            FieldType::Type type = TableField::typeFromText(type_str);
            for(const String& s : fields.split(",", String::SkipEmptyParts)){
                FieldAssociation& fa = table_association[s];
                fa.type = type;
                fa.new_name = s;
            }

            pos_sub += re_sub.matchedLength();
        }

        for(const String& table_name :  re.cap(1).split(",", String::SkipEmptyParts)){
            TableAssociation &ta = res[table_name];
            ta.unite(table_association);
        }
        pos += re.matchedLength();
    }
    return res;
}


