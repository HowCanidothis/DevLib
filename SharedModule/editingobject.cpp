#include "editingobject.h"

EditingObject::EditingObject()
    : ForceDirty(false)
    , m_isActive(false)
    , m_isDirty(false)
{
    m_isDirty.ConnectFrom(CONNECTION_DEBUG_LOCATION, ForceDirty, [this](bool forceDirty) { return forceDirty ? true : m_isDirty.Native(); });
}

void EditingObject::SetParent(EditingObject* parent)
{
    m_parentConnections.clear();

    parent->OnEnteredEditingMode.Connect(this, [this]{
        OnEnteredEditingMode();
    }).MakeSafe(m_parentConnections);

    parent->OnLeavedEditingMode.Connect(this, [this]{
        OnLeavedEditingMode();
    }).MakeSafe(m_parentConnections);

    parent->OnAboutToBeSaved.Connect(this, [this]{
        Save();
    }).MakeSafe(m_parentConnections);

    parent->OnAboutToBeDiscarded.Connect(this, [this]{
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

