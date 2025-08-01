#include "importexport.h"
#include "SharedModule/builders.h"

#include <QCryptographicHash>

#ifdef SHARED_LIB_ADD_UI
#include <QClipboard>
#include <QApplication>
#endif

ImportExportFilterExtensionsBuilder::ImportExportFilterExtensionsBuilder(bool addZip)
    : m_addAllTypes([](QStringList&){})
{
    if(addZip) {
        m_addZip = [this](const ExtensionValue& value){
            m_extensions.append(std::make_pair(QString("Archive %1").arg(value.first.isEmpty() ? value.second.toUpper() : value.first), QString("%1.zip").arg(value.second)));
        };
    } else {
        m_addZip = [](const ExtensionValue&){};
    }
}

SP<ImportExportBufferSource> ImportExportBufferSource::CreateFromClipboard()
{
#ifdef SHARED_LIB_ADD_UI
    THREAD_ASSERT_IS_MAIN()
    QClipboard* clipboard = qApp->clipboard();
    static QByteArray text;
    text = clipboard->text().toUtf8();
    return ::make_shared<ImportExportBufferSource>("txt", "Clipboard", &text);
#endif
    return nullptr;
}

ImportExportFilterExtensionsBuilder& ImportExportFilterExtensionsBuilder::AddAllTypes(bool enable)
{
    if(enable) {
        m_addAllTypes = [this](QStringList& result){
            result.append(QString("All Types (%1)").arg(StringBuilder().Join(' ', m_extensions, [](const auto& it){
                return QString("*.%1").arg(it->second);
            })));
        };
    } else {
        m_addAllTypes = [](QStringList&){};
    }
    return *this;
}

ImportExportFilterExtensionsBuilder& ImportExportFilterExtensionsBuilder::AddExtension(const QString& extension)
{
    AddExtension(QString(), extension);
    return *this;
}

ImportExportFilterExtensionsBuilder& ImportExportFilterExtensionsBuilder::AddExtensions(const QStringList& extensions)
{
    for(const auto& extension : extensions) {
        AddExtension(extension);
    }
    return *this;
}

ImportExportFilterExtensionsBuilder& ImportExportFilterExtensionsBuilder::AddExtensions(const QSet<Name>& extensions)
{
    for(const auto& extension : extensions) {
        AddExtension(extension.AsString());
    }
    return *this;
}

ImportExportFilterExtensionsBuilder& ImportExportFilterExtensionsBuilder::AddExtension(const QString& label, const QString& extension)
{
    m_extensions.append(std::make_pair(label, extension));
    m_addZip(m_extensions.constLast());
    return *this;
}

QStringList ImportExportFilterExtensionsBuilder::Result()
{
    ContainerBuilder<QStringList> result;
    m_addAllTypes(result);
    result.Append(m_extensions, [](const auto& value){
        const auto& label = value->first;
        const auto& ext = value->second;
        return QString("%1 (*.%2)").arg(label.isEmpty() ? ext.toUpper() : label, ext);
    });
    return std::move(result);
}

ThreadPool& ImportExport::threadPool()
{
    static ThreadPool result;
    return result;
}

QString ImportExport::StandardImportingString()
{
    return tr("Importing %1...");
}

QString ImportExport::StandardExportingString()
{
    return tr("Exporting to %1...");
}

AsyncResult ImportExport::async(const FAction& function, const EPriority priority)
{
    return threadPool().PushTask(function, priority);
}

QString ImportExportSource::ParseExtension(const QString& path)
{    
    QString extension;
    QString fileName;
    qint32 index(0);

    thread_local QRegExp m_parseExtension(R"([^\/\\]+)");
    thread_local QRegExp m_parseExtensionEx(R"(([^\.]*)\.(.*))");

    while((index = m_parseExtension.indexIn(path, index)) != -1) {
        fileName = m_parseExtension.cap();
        index += m_parseExtension.matchedLength();
    }
    if(m_parseExtensionEx.indexIn(fileName, 0) != -1) {
        extension = m_parseExtensionEx.cap(2);
    }

    return extension;
}

void ImportExportSource::SetError(const QVariant& error)
{
    StandardProperties.Error = error;
}

void ImportExportFormatFactory::Register(const FormatDelegates& delegates, const FHandler& handler)
{
    for(auto it(delegates.begin()), e(delegates.end()); it != e; ++it) {
        auto foundIt = m_factory.find(it.key());
        if(foundIt == m_factory.end()) {
            foundIt = m_factory.insert(it.key(), Container());
        }
        foundIt.value().append(std::make_pair(it.value(), handler));
    }
}

void ImportExportFormatFactory::Register(const QSet<Name>& extensions, const FHandler& handler)
{
    for(const auto& extension : extensions) {
        auto foundIt = m_factory.find(extension);
        if(foundIt == m_factory.end()) {
            foundIt = m_factory.insert(extension, Container());
        }
        foundIt.value().append(std::make_pair([](const ImportExportSourcePtr&) { return true; }, handler));
    }
}

AsyncResult ImportExportFormatFactory::Import(const QList<ImportExportSourcePtr>& sources) const {
    FutureResult future;

    auto handleExtension = [&](const Name& extension, const ImportExportSourcePtr& source) {
        auto foundIt = m_factory.find(extension);
        if(foundIt != m_factory.end()) {
            for(const auto& delegate : foundIt.value()) {
                if(delegate.first(source)) {
                    future += delegate.second(source);
                    return true;
                }
            }
        }
        return false;
    };

    for(const auto& source : sources) {
        auto extension = source->GetExtension();
        if(!handleExtension(extension, source)) {
            // try most right
            auto extensionParts = extension.AsString().split(".");
            if(extensionParts.size() > 1) {
                Name shortExtension(extensionParts.constLast());
                source->SetExtension(shortExtension);
                if(!handleExtension(shortExtension, source)) {
                    source->SetError(tr("%1 extension is not supported").arg(extension.AsString()));
                }
            } else {
                source->SetError(tr("%1 extension is not supported").arg(extension.AsString()));
            }
        }
    }
    return future.ToAsyncResult();
}

AsyncResult ImportExport::WriteSecuritySum(const ImportExportSourcePtr& source, const AsyncResult& res)
{
    if(source->StandardProperties.SecurityKeyWord.isEmpty()) {
        return res;
    }
    AsyncResult result;
    res.Then([result, source](bool r) {
        if(r) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            auto* device = source->GetDevice();
            auto p = device->pos();
            device->seek(0);
            hash.addData(device);
            hash.addData(source->StandardProperties.SecurityKeyWord);
            auto hr = hash.result();
            auto ws = QString(R"(<hashsum="%1"/>)").arg(QString(hr.toHex()));
            device->seek(p);
            device->write(ws.toLatin1());
        }
        result.Resolve(r);
    });
    return result;
}

bool ImportExport::CheckSecuritySum(const ImportExportSourcePtr& source)
{
    auto bytearray = source->GetDevice()->readAll();
    QCryptographicHash hash(QCryptographicHash::Md5);
    auto fsize = QCryptographicHash::hashLength(QCryptographicHash::Md5) * 2  + 13;
    auto foffset = bytearray.size() - fsize;
    if(foffset <= 0) {
        return false;
    }
    hash.addData(bytearray.cbegin(), foffset);
    hash.addData(source->StandardProperties.SecurityKeyWord);

    auto hr = QString(R"(<hashsum="%1"/>)").arg(QString(hash.result().toHex()));
    auto rhr = QByteArray(bytearray.cbegin() + foffset, fsize);

    source->GetDevice()->seek(0);
    bool result = hr == rhr;
    return result;
}

SerializerXmlVersion ImportExport::ReadVersionXML(QIODevice* device, const QChar& separator) {
    QXmlStreamReader xmlReader;
    if(!device->isOpen()) {
        if(!device->open(QIODevice::ReadOnly)) {
            return SerializerXmlVersion();
        }
    }
    xmlReader.setDevice(device);
    SerializerXmlReadBuffer buffer(&xmlReader);
    auto result = buffer.ReadVersion(separator);
    device->close();
    return result;
}

SerializerXmlVersion ImportExport::ReadVersionJson(QIODevice* device, const QChar& separator) {
    if(!device->isOpen()) {
        if(!device->open(QIODevice::ReadOnly)) {
            return SerializerXmlVersion();
        }
    }
    auto data = device->readAll();
    auto fsize = QCryptographicHash::hashLength(QCryptographicHash::Md5) * 2  + 13;
    auto foffset = data.size() - fsize;
    QByteArray hs(data.begin() + foffset);
    if(hs.startsWith("<hashsum=")) {
        data.resize(foffset);
    }
    auto doc = QJsonDocument::fromJson(data);
    auto object = doc.object();
    SerializerJsonReadBuffer reader(&object);
    auto result = reader.ReadVersion(separator);
    device->close();
    return result;
}

SerializerVersion ImportExport::ReadVersion(QIODevice* device) {
    SerializerReadBuffer buffer(device);
    if(!device->isOpen()) {
        if(!device->open(QIODevice::ReadOnly)) {
            return SerializerVersion();
        }
    }
    auto result = buffer.ReadVersion();
    device->close();
    return result;
}
