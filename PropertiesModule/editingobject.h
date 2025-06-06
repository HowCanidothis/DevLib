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

    virtual void Initialize() = 0;

    virtual QByteArray Serialize(const void* o, const DescSerializationXMLWriteParams& params) const = 0;
    virtual void Serialize(SerializerXmlWriteBuffer& writeBuffer, const void* o) const = 0;
    virtual void Deserialize(const QByteArray& array, void* o, const DescSerializationXMLReadParams& params) const = 0;
    virtual void Deserialize(SerializerXmlReadBuffer& buffer, void* o) const = 0;

    virtual QJsonObject SerializeJson(const void* o, const DescSerializationXMLWriteParams& params) const = 0;
    virtual void DeserializeJson(const QJsonObject& array, void* o, const DescSerializationXMLReadParams& params) const = 0;
};

class CompareXmlObject
{
public:
    CompareXmlObject(const QByteArray& data1, const QByteArray& data2);

    static std::pair<QString, qint64> uploadData(const QString& id, const QByteArray& data, const QByteArray& text);
    static std::pair<QByteArray, QByteArray> downloadData(const std::pair<QString,qint64>& fileInfo);

    static DescSerializationXMLWriteParams createCompareParams(qint32 serializationMode);
    static qint32 createSaveRestoreMode(qint32 serializationMode);

private:
    ScopedPointer<class QProcess> m_compareProcess;
};

template<class T, qint32 SerializationMode>
class TEditingObjectComponent : public EditingObjectComponentBase
{
public:
    TEditingObjectComponent(T* object, bool testChanged = true)
    {
        m_initialize = [object, testChanged, this]{
            auto& editObject = object->EditObject;
            editObject.DiffRequest.Connect(CONNECTION_DEBUG_LOCATION, [this, object, testChanged]{
                if(!testChanged) {
                    return;
                }
                auto currentData = Serialize(object, CompareXmlObject::createCompareParams(SerializationMode));
                m_compareObject = new CompareXmlObject(load().second, currentData);
            });
            editObject.OnEnteredEditingMode.Connect(CONNECTION_DEBUG_LOCATION, [this, object, testChanged]{
                save(object, testChanged);
                if(testChanged) {
                    object->OnChanged += { this, [object, this]{
                        auto currentStorageData = Serialize(object, CompareXmlObject::createCompareParams(SerializationMode));
                        object->EditObject.SetDirty(load().second != currentStorageData);
                    }};
                } else {
                    object->OnChanged += { this, [object, this]{
                        object->EditObject.SetDirty(true);
                    }};
                }
            });
            editObject.OnAboutToBeSaved.Connect(CONNECTION_DEBUG_LOCATION, [this, object, testChanged]{
                save(object, testChanged);
            });
            editObject.OnAboutToBeDiscarded.Connect(CONNECTION_DEBUG_LOCATION, [this, object, testChanged]{
                if(object->EditObject.IsDirty()){
                    auto f = load().first;
                    if(!f.isEmpty()) {
                        DeSerializeFromArray(load().first, *object, CompareXmlObject::createSaveRestoreMode(SerializationMode));
                        if(!testChanged) {
                            object->OnChanged.OnFirstInvoke([=]{
                                //auto currentStorageData = Serialize(object, m_writeParams);
                                object->EditObject.SetDirty(false);
                            });
                        }
                    }
                }
            });
            editObject.OnLeavedEditingMode.Connect(CONNECTION_DEBUG_LOCATION, [this, object]{
                object->OnChanged -= this;
                m_compareObject = nullptr;
            });
        };

    }

    void Initialize() override
    {
        if(m_initialize == nullptr) {
            return;
        }
        m_initialize();
        m_initialize = nullptr;
    }

    QJsonObject SerializeJson(const void* o, const DescSerializationXMLWriteParams& params) const override
    {
        return SerializeToJsonObject(QString(), *(T*)o, params);
    }

    void DeserializeJson(const QJsonObject& array, void* o, const DescSerializationXMLReadParams& params) const override
    {
        DeSerializeFromJson(QString(), array, *(T*)o, params);
    }

    QByteArray Serialize(const void* o, const DescSerializationXMLWriteParams& params) const override
    {
        return SerializeToXML("Object", *(T*)o, params);
    }

    void Serialize(SerializerXmlWriteBuffer& writeBuffer, const void* o) const override
    {
        writeBuffer << writeBuffer.Sect("Object", *(T*)o);
    }

    void Deserialize(const QByteArray& array, void* o, const DescSerializationXMLReadParams& params) const override
    {
        DeSerializeFromXML("Object", array, *(T*)o, params);
    }

    void Deserialize(SerializerXmlReadBuffer& buffer, void* o) const override
    {
        buffer << buffer.Sect("Object", *(T*)o);
    }

private:
    void save(const T* object, bool testChanged)
    {
        m_fileInfo = CompareXmlObject::uploadData(object->GetEditId(), SerializeToArray(*object, CompareXmlObject::createSaveRestoreMode(SerializationMode)), testChanged ? Serialize(object, CompareXmlObject::createCompareParams(SerializationMode)) : QByteArray());
    }

    std::pair<QByteArray, QByteArray> load()
    {
        return CompareXmlObject::downloadData(m_fileInfo);
    }

private:
    std::pair<QString, qint64> m_fileInfo;
    ScopedPointer<CompareXmlObject> m_compareObject;
    FAction m_initialize;
};

#endif // EDITINGOBJECT_H
