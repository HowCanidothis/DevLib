#include "editingobject.h"

EditingObject::EditingObject()
    : ForceDirty(false)
    , m_isActive(false)
    , m_isDirty(false)
{
    m_isDirty.ConnectFrom(CONNECTION_DEBUG_LOCATION, [this](bool forceDirty) { return forceDirty ? true : m_isDirty.Native(); }, ForceDirty);
    adapters::ResetThread(ForceDirty, m_isDirty);
}

void EditingObject::SetParent(EditingObject* parent)
{
    m_parentConnections.clear();

    parent->OnEnteredEditingMode.Connect(CDL, [this]{
        SetActive(true);
    }).MakeSafe(m_parentConnections);

    parent->OnAboutToBeSaved.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        Save();
    }).MakeSafe(m_parentConnections);

    parent->OnAboutToBeDiscarded.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        Discard();
    }).MakeSafe(m_parentConnections);
}

bool EditingObject::SetActive(bool active, const QVector<DirtyMode>& disableModes)
{
    if(m_isActive == active) {
        return true;
    }

    if(active) {
        m_isActive = true;
        OnEnteredEditingMode();
    } else {
        if(IsDirty()) {
            DirtyMode ok = DirtyModeDiscard;
            OnLeavingDirtyEditingMode(&ok, disableModes);
            switch(ok) {
            case DirtyModeSave: Save(); break;
            case DirtyModeDiscard: Discard(); break;
            default: return false;
            }
        }
        m_isActive = false;
        OnLeavedEditingMode();
    }
    return true;
}

void EditingObject::Save()
{
    if(m_isDirty){
        OnAboutToBeSaved();

        OnLeavedEditingMode();
        OnEnteredEditingMode();

        SetDirty(false);
    }
}

void EditingObject::Discard()
{
    if(m_isDirty){
        OnAboutToBeDiscarded();

        SetDirty(false);
        OnDiscarded();
    }
}

void EditingObject::SetDirty(bool dirty)
{
    if(!dirty && !ForceDirty) {
        m_isDirty = false;
    } else {
        m_isDirty = true;
    }
}

CompareXmlObject::CompareXmlObject(const QByteArray& data1, const QByteArray& data2)
{
    auto& pathSettings = SharedSettings::GetInstance().PathSettings;
    auto oldFilePath = pathSettings.TempDir.filePath("old.txt");
    auto newFilePath = pathSettings.TempDir.filePath("new.txt");

    QFile newFile(newFilePath);
    newFile.open(QFile::WriteOnly);
    newFile.write(data2);

    QFile oldFile(oldFilePath);
    oldFile.open(QFile::WriteOnly);
    oldFile.write(data1);

    m_compareProcess = new QProcess();
    m_compareProcess->start(pathSettings.TextComparatorApplicationPath, QStringList() << oldFilePath << newFilePath);
}

std::pair<QString, qint64> CompareXmlObject::uploadData(const QString& id, const QByteArray& data, const QByteArray& text)
{
    static QDir appDir = []{
        QDir result(SharedSettings::GetInstance().PathSettings.TempDir);
        result.mkdir("sc");
        result.cd("sc");
        auto pid = QString::number(qApp->applicationPid());
        result.mkdir(pid);
        result.cd(pid);
        return result;
    }();
    std::pair<QString, qint64> result;
    result.first = appDir.absoluteFilePath(id);
    QFile f(result.first);
    if(f.open(QFile::WriteOnly)) {
        result.second = f.write(data);
        f.write(text);
        f.close();
    }
    return result;
}

std::pair<QByteArray, QByteArray> CompareXmlObject::downloadData(const std::pair<QString, qint64>& fileInfo)
{
    std::pair<QByteArray, QByteArray> result;
    QFile f(fileInfo.first);
    if(f.open(QFile::ReadOnly)) {
         result.first = f.read(fileInfo.second);
         result.second = f.readAll();
    }
    return result;
}

DescSerializationXMLWriteParams CompareXmlObject::createCompareParams(qint32 serializationMode)
{
    return DescSerializationXMLWriteParams(serializationMode | SerializationMode_Comparison).SetAutoFormating(true)
            .SetTextConverterContext(TextConverterContext().SetDoublePrecision(6).SetFloatPrecision(6));
}

qint32 CompareXmlObject::createSaveRestoreMode(qint32 serializationMode)
{
    return serializationMode | SerializationMode_InvokeProperties | SerializationMode_MinMaxProperties;
}

