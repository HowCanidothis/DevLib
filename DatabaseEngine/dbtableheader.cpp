#include "dbtableheader.h"

#include "dbmodificationrule.h"
#include "Core/memory/dbmemory.h"
#include "Core/memory/dbmemorydeclarations.h"
#include "Core/private_objects/dbtablefielddelegate.h"

DB_BEGIN_NAMESPACE

DbTableHeader::DbTableHeader(DbMemory *memory, MDbTableHeaderDeclaration* declaration) :
    m_memory(memory),
    m_declaration(declaration),
    m_size(0)
{}

void DbTableHeader::updatePlainData(const DbAddressMap& addressMap)
{
    m_declaration->updatePlainData(addressMap);
    for(MDbTableField* field : m_fields) {
        field->updatePlainData(addressMap);
    }
}

void DbTableHeader::edit(const DbModificationRule& rule)
{
    qint32 resultFieldCount = 0;

    for(const DbCopyFieldDelegate& delegate : rule.m_copyDelegates) {
        if(delegate.DestinationOffset != 0) {
            resultFieldCount++;
        }
    }

    auto oldFields = m_fields;
    reset();
    qint32 dif = resultFieldCount - oldFields.Size();
    if(dif > 0) {
        while(dif--) {
            MDbTableField* field = m_memory->Allocate<MDbTableField>(DbMemoryFragment::ReasonTableFieldDeclaration, m_declaration->GetID(), DbTableFieldType::Boolean, "");
            oldFields.Append(field);
        }
    } else if(dif < 0) {
        for(MDbTableField* fieldToRemove : adapters::range(oldFields.begin() + resultFieldCount, oldFields.end())) {
            fieldToRemove->Free();
        }
    }
    auto it = oldFields.begin();
    for(const DbCopyFieldDelegate& delegate : rule.m_copyDelegates) {
        if(delegate.DestinationOffset != 0) {
            MDbTableField* field = *it;
            field->SetType(delegate.FieldType);
            field->Name = delegate.Name;
            field->DefaultValue = delegate.DefaultValue;
            addField(field);
            it++;
        }
    }
    m_declaration->FieldsCount = resultFieldCount;
}

void DbTableHeader::setUsage(qint32 usage)
{
    m_declaration->Usage = usage;
}

qint32 DbTableHeader::GetUsage() const
{
    return this->m_declaration->Usage;
}

void DbTableHeader::SetName(const QString& name)
{
    this->m_declaration->Name = name;
}

const DbString& DbTableHeader::GetName() const
{
    return this->m_declaration->Name;
}

void DbTableHeader::reset()
{
    m_fields.Clear();
    m_complexFieldsOffsets.Clear();
    m_fieldDelegates.Clear();
    m_defaultValues.clear();
    m_size =0;
}

void DbTableHeader::AddField(DbTableFieldType type, const QString& name, const QString& defaultValue)
{
    MDbTableField* field = m_memory->Allocate<MDbTableField>(DbMemoryFragment::ReasonTableFieldDeclaration, m_declaration->GetID(), type, name);
    field->DefaultValue = defaultValue;
    addField(field);
}

void DbTableHeader::Edit(MDbTableField* tableField, DbTableFieldType type, const QString& name, const QString& defaultValue)
{
    Q_ASSERT(!tableField->IsPrimaryKey());
    Q_ASSERT_X(!IsUsed(), "TableHead::edit", "Function function is restricted when head is in using, use Database::createModifyRule() instead");
    Array<MDbTableField*> old = this->m_fields;
    tableField->Type = type;
    tableField->Name = name;
    tableField->DefaultValue = defaultValue;
    this->reset();
    for(MDbTableField* tf : old) {
        addField(tf);
    }
}

void DbTableHeader::Remove(MDbTableField* field)
{
    Q_ASSERT(!field->IsPrimaryKey());
    Q_ASSERT_X(!IsUsed(), "TableHead::remove", "Function is restricted when head is in using, use Database::createModifyRule() instead");
    Array<MDbTableField*> old = this->m_fields;
    this->reset();
    field->Free();
    old.Remove(field);
    for(MDbTableField *tf : old) {
        addField(tf);
    }
}

void DbTableHeader::InitializeDefaultValues(CastablePtr& row) const
{
    for(auto offset : m_complexFieldsOffsets) {
        new ((row + offset).AsPtr<void>()) MDbArrayBase(0);
    }
    for(const DbDefaultValuesSetter& setter : m_defaultValues) {
        setter(row);
    }
}

#define DECL_SWITCH(CppType, First) \
    case First: stream << "\tciterator findFirst_" + field_names[f->Pos] + "(const " + DbTableFieldTypeHelper<CppType>::valueName() + "& s){ this->sort_" + field_names[f->Pos] + "(); return std::lower_bound(this->begin(), this->end(), s, [](const " + struct_name + " &s, const " + DbTableFieldTypeHelper<CppType>::valueName() + "& v){ return s." + field_names[f->Pos] + " < v; }); }\n"; break;
void DbTableHeader::CreateConcreteTableDataHeader(QTextStream &stream,const OutCreateConcreteTableDataHeaderNameGenerator *name_gen) const
{
    QString name_space = name_gen->NameSpace;
    QString upper = name_gen->ConcreteTableDataName.toUpper();
    QString struct_name = name_gen->StructName;
    QString HHH = name_space.toUpper() + "_" + upper + "_H";
    QString concreteTableData_name = name_gen->ConcreteTableDataName;
    QString* types = new QString[m_fields.Size()];
    QString* field_names = new QString[m_fields.Size()];

    for(qint32 i(0); i < m_fields.Size(); i++) {
        types[i] = MDbTableField::TypeToString(m_fields.At(i)->Type);
        field_names[i] = m_fields.At(i)->Name.ToString();
    }

    stream << "//this part of code is autogenerated by TableHead::createConcreteTableDataHeader function\n";
    stream << "#ifndef " + HHH + "\n";
    stream << "#define " + HHH + "\n";
    stream << "#include \"table.h\"\n";
    stream << "namespace " + name_space + "{\n";
    //======================Struct==================
    stream << "#pragma pack(1)\n";
    stream << "struct " + struct_name + "\n";
    stream << "{\n";
    for(const MDbTableField* f : m_fields)
        stream << "\t" + types[f->Pos] + " " + field_names[f->Pos] + ";\n";
    stream << "\n\t" << struct_name << "& operator=(const " + struct_name + "& other) {\n";
    for(const MDbTableField* f : m_fields){
        if(f->Name != "id")
            stream << "\t\tthis->" + field_names[f->Pos] + " = other." + field_names[f->Pos] + ";\n";
    }
    stream << "\t\treturn *this;\n";
    stream << "\t}\n";
    stream << "};\n";
    stream << "#pragma pack()\n";
    //======================ConcreteData==================
    stream << "class " + concreteTableData_name + ": public TableSourceT<" + struct_name + ">\n";
    stream << "{\n";
    stream << "\ttypedef TableSourceT<" + struct_name + "> PTableSource;\n";
    stream << "public:\n";
    //set
    for(MDbTableField* f : m_fields){
        if(f->Name != "id")
            stream << "\t" + types[f->Pos] + "& set_" + field_names[f->Pos] + "(const " + struct_name + "& s){ this->update(&s, " + QString::number(f->Pos) + "); return CastablePtr(&s." + field_names[f->Pos] + ").as<" + types[f->Pos] + ">(); }\n";
    }
    stream << "\n";
    //find
    for(MDbTableField* f : m_fields)
    {
        switch (f->Type) {
        DB_FOREACH_COMPLEX_FIELDS(DECL_SWITCH);
        default:
            stream << "\tciterator findFirst_" + field_names[f->Pos] + "(const " + types[f->Pos] + "& s){ this->sort_" + field_names[f->Pos] + "(); return std::lower_bound(this->begin(), this->end(), s, [](const " + struct_name + " &s, const " + types[f->Pos] + "& v){ return s." + field_names[f->Pos] + " < v; }); }\n";
            break;
        }
    }
    stream << "\n";
    //sort
    stream << "\tvoid sort_id(){ PTableSource::toSource(); }\n";
    for(MDbTableField* f : m_fields){
        if(f->Name != "id")
            stream << "\tvoid sort_" + field_names[f->Pos] + "(){ PTableSource::sort(" + QString::number(f->Pos) + "); }\n";
    }
    stream << "\n";
    stream << "\tciterator at(qint32 j){ return citerator(this->current->begin() + j);  }\n";
    stream << "\tciterator begin() {  return citerator(this->current->begin());  }\n";
    stream << "\tciterator end() { return citerator(this->current->end());  }\n";
    stream << "};\n";
    stream << "typedef " + concreteTableData_name + "::citerator " + concreteTableData_name + "It;\n";
    for(const QString& n : name_gen->VirtualConcreteTableDataNames)
        stream << "typedef " + concreteTableData_name + "::citerator " + n + "It;\n";
    for(const QString& n : name_gen->VirtualConcreteTableDataNames)
        stream << "typedef " + concreteTableData_name + " " + n + ";\n";
    for(const QString& n : name_gen->VirtualStructNames)
        stream << "typedef " + struct_name + " " + n + ";\n";
    stream << "}\n";
    stream << "#endif\n";

    delete [] types;
    delete [] field_names;
}

qint32 DbTableHeader::IndexOf(const QString& name) const
{
    Array<MDbTableField*>::const_iterator res = std::find_if(m_fields.begin(), m_fields.end(), [&](const MDbTableField *f){ return f->Name == name; });
    if(res == m_fields.end()) return -1;
    return std::distance(m_fields.begin(), res);
}

const MDbTableField*DbTableHeader::GetField(const QString& name) const
{
    qint32 indexOf = IndexOf(name);
    if(indexOf != -1) {
        return m_fields.At(indexOf);
    }
    return nullptr;
}

void DbTableHeader::setBegin(MDbTableHeaderDeclaration* declaration)
{
    m_fieldDelegates.Clear(); // old deleted here
    m_complexFieldsOffsets.Clear();
    m_defaultValues.clear();

    this->m_declaration = declaration;
    m_fields.Resize(declaration->FieldsCount);
    m_fieldDelegates.Resize(declaration->FieldsCount);
}

void DbTableHeader::setField(MDbTableField* field)
{
    m_size += field->Size;
    m_fields.At(field->Pos) = field;
    m_fieldDelegates.At(field->Pos) = new DbTableFieldDelegate(field);
    if(!field->DefaultValue.IsEmpty()) {
        m_defaultValues.append(field->CreateDefaultValueSetter());
    }
}

void DbTableHeader::setEnd()
{
    qint32 size=0;
    for(DbTableFieldDelegate* delegate : m_fieldDelegates) {
        if(delegate->IsComplex()) {
            m_complexFieldsOffsets.Append(size);
        }
        size += delegate->GetSize();
    }
}

void DbTableHeader::free()
{
    m_declaration->Free();
    for(MDbTableField* field : m_fields) {
        field->Free();
    }
}

DbTableHeader::~DbTableHeader()
{

}

void DbTableHeader::addField(MDbTableField* field)
{
    field->Pos = m_fields.Size();
    field->Offset = m_size;

    DbTableFieldDelegate* delegate = new DbTableFieldDelegate(field);
    if(delegate->IsComplex()) {
        m_complexFieldsOffsets.Append(m_size);
    }

    if(!field->DefaultValue.IsEmpty()) {
        m_defaultValues.append(field->CreateDefaultValueSetter());
    }
    m_fields.Append(field);
    m_fieldDelegates.Append(delegate);
    m_size += field->Size;
    m_declaration->FieldsCount++;
}

void DbTableHeader::addPrimaryKey()
{
    AddField<qint32>("id");
    m_fields.Last()->m_isPrimaryKey = true;
}

id_t DbTableHeader::GetID() const
{
    return this->m_declaration->GetID();
}

void DbTableHeader::IncrementUsage()
{
    m_declaration->Usage++;
}

void DbTableHeader::DecrementUsage()
{
    m_declaration->Usage--;
}

DB_END_NAMESPACE
