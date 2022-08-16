#include "importexport.h"

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

ImportExportFilterExtensionsBuilder& ImportExportFilterExtensionsBuilder::AddAllTypes(bool enable)
{
    if(enable) {
        m_addAllTypes = [this](QStringList& result){
            result.append(QString("All Types (%1)").arg(lq::Join(' ', lq::Select<QString>(m_extensions, [](const ExtensionValue& value){
                return QString("*.%1").arg(value.second);
            }))));
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
    QStringList result;
    m_addAllTypes(result);
    lq::Join(m_extensions, [&](const ExtensionValue& value){
        const auto& label = value.first;
        const auto& ext = value.second;
        result.append(QString("%1 (*.%2)").arg(label.isEmpty() ? ext.toUpper() : label, ext));
    });
    return result;
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
    for(const auto& source : sources) {
        auto foundIt = m_factory.find(source->GetExtension());
        if(foundIt != m_factory.end()) {
            bool found = false;
            for(const auto& delegate : foundIt.value()) {
                if(delegate.first(source)) {
                    future += delegate.second(source);
                    found = true;
                    break;
                }
            }
            if(!found) {
                source->SetError(tr("File format is not supported"));
            }
        } else {
            source->SetError(tr("%1 extension is not supported").arg(source->GetExtension().AsString()));
        }
    }
    return future.ToAsyncResult();
}
