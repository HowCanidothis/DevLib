#ifndef EDITINGOBJECT_H
#define EDITINGOBJECT_H

#include "localproperty.h"

using EditingObjectPtr = SharedPointer<class EditingObject>;

class EditingObject
{
public:
    enum DirtyMode{
        DirtyModeSave = 0,
        DirtyModeDiscard,
        DirtyModeCancel
    };

    EditingObject();

    void SetParent(EditingObject* parent);
    bool SetActive(bool active, const QVector<DirtyMode>& disableModes = QVector<DirtyMode>());
    void Save();
    void Discard();

    bool IsDirty() const { return m_isDirty; }
    void SetDirty(bool dirty = true);
    bool IsActive() const { return m_isActive; }
    Dispatcher& OnDirtyChanged() { return m_isDirty.OnChanged; }

    LocalPropertyBool ForceDirty;

    LocalPropertyErrorsContainer HasErrors;

    Dispatcher DiffRequest;
    Dispatcher OnEnteredEditingMode;
    Dispatcher OnLeavedEditingMode;
    CommonDispatcher<DirtyMode*, const QVector<DirtyMode>&> OnLeavingDirtyEditingMode;

    Dispatcher OnAboutToBeSaved;
    Dispatcher OnAboutToBeDiscarded;
    Dispatcher OnDiscarded;

private:
    DispatcherConnectionsSafe m_parentConnections;
    bool m_isActive;
    LocalPropertyBool m_isDirty;
};


class EditingObjectComponentBase
{
public:
    virtual ~EditingObjectComponentBase() {}

    virtual QByteArray Serialize(const void* o) const = 0;
    virtual void Serialize(SerializerXmlWriteBuffer& writeBuffer, const void* o) const = 0;
    virtual void Deserialize(const QByteArray& array, void* o) const = 0;
    virtual void Deserialize(SerializerXmlReadBuffer& buffer, void* o) const = 0;
};

class CompareXmlObject
{
public:
    CompareXmlObject(const QByteArray& data1, const QByteArray& data2);

private:
    ScopedPointer<class QProcess> m_compareProcess;
};

template<class T, qint32 SerializationMode>
class TEditingObjectComponent : public EditingObjectComponentBase
{
public:
    TEditingObjectComponent(T* object)
    {
        auto& editObject = object->EditObject;

        editObject.DiffRequest.Connect(CONNECTION_DEBUG_LOCATION, [this, object]{
            auto currentData = Serialize(object);
            m_compareObject = new CompareXmlObject(m_storageData, currentData);
        });
        editObject.OnEnteredEditingMode.Connect(CONNECTION_DEBUG_LOCATION, [this, object]{
            m_storageData = Serialize(object);

            object->OnChanged += { this, [object, this]{
                auto currentStorageData = Serialize(object);
                object->EditObject.SetDirty(m_storageData != currentStorageData);
            }};
        });
        editObject.OnAboutToBeSaved.Connect(CONNECTION_DEBUG_LOCATION, [this, object]{
            m_storageData = Serialize(object);
        });
        editObject.OnAboutToBeDiscarded.Connect(CONNECTION_DEBUG_LOCATION, [this, object]{
            if(object->EditObject.IsDirty()){
                Deserialize(m_storageData, object);
            }
        });
        editObject.OnLeavedEditingMode.Connect(CONNECTION_DEBUG_LOCATION, [this, object]{
            object->OnChanged -= this;
        });
    }

    QByteArray Serialize(const void* o) const override
    {
        return SerializeToXML("Object", *(T*)o, DescSerializationXMLWriteParams(SerializationMode)
                              .SetAutoFormating(true));
    }


    void Serialize(SerializerXmlWriteBuffer& writeBuffer, const void* o) const override
    {
        writeBuffer << writeBuffer.Sect("Object", *(T*)o);
    }

    void Deserialize(const QByteArray& array, void* o) const override
    {
        DeSerializeFromXML("Object", array, *(T*)o, DescSerializationXMLReadParams(SerializationMode));
    }

    void Deserialize(SerializerXmlReadBuffer& buffer, void* o) const override
    {
        buffer << buffer.Sect("Object", *(T*)o);
    }

private:
    QByteArray m_storageData;
    ScopedPointer<CompareXmlObject> m_compareObject;
};

#endif // EDITINGOBJECT_H
