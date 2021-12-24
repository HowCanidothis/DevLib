#include "importexport.h"

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
