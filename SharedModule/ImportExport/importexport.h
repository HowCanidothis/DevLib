#ifndef IMPORT_EXPORT_H
#define IMPORT_EXPORT_H


struct DescImportExportSourceParams
{
    enum EMode
    {
        Save,
        Load,
        LoadSingle
    };

    QString Label;
    QString FileName;
    EMode Mode;
    QStringList Filters;
    QString DefaultSuffix;
    qint32 ExpectedExtensionsCount;

    DescImportExportSourceParams(EMode mode)
        : Mode(mode)
        , ExpectedExtensionsCount(2)
    {}

    DescImportExportSourceParams& SetLabel(const QString& label) { Label = label; return *this; }
    DescImportExportSourceParams& SetFileName(const QString& fileName) { FileName = fileName; return *this; }
    DescImportExportSourceParams& SetFilters(const QStringList& filters) { Filters = filters; return *this; }
    DescImportExportSourceParams& SetDefaultSuffix(const QString& defaultSuffix) { DefaultSuffix = defaultSuffix; return *this; }
    DescImportExportSourceParams& SetExpectedExtensionsCount(qint32 count) { ExpectedExtensionsCount = count; return *this; }
};

struct ImportExportSourceStandardProperties
{
    QVariant Error;
    bool IsMultithread = false;
    bool IsCompressed = false;
    bool IsAutoMatching = false;
    bool IsExportHeader = true;
    bool IsMuted = false;
    std::function<qint8 (const QList<QString>&)> ImportTableStringHandler;
    std::function<qint8 (const QList<QVector<QVariant>>&)> ImportTableDataHandler;
};

using ImportExportSourceStandardPropertiesPtr = SharedPointer<ImportExportSourceStandardProperties>;
Q_DECLARE_METATYPE(ImportExportSourceStandardPropertiesPtr)

class ImportExportSource
{
public:
    virtual ~ImportExportSource() {}

    virtual QIODevice* GetDevice() = 0;
    virtual const Name& GetExtension() const = 0;
    virtual const QString& GetSourceName() const = 0;

    template<class T>
    static QList<SharedPointer<ImportExportSource>> CreateSources(const DescImportExportSourceParams& params);

    static QString ParseExtension(const QString& path);

    StandardVariantPropertiesContainer Properties;
    ImportExportSourceStandardProperties StandardProperties;

    void SetError(const QVariant& error);
};

using ImportExportSourcePtr = SharedPointer<ImportExportSource>;

class ImportExportBufferSource : public ImportExportSource
{
public:
    ImportExportBufferSource(const Name& extension, const QString& sourceName, QByteArray* array)
        : m_buffer(array)
        , m_extension(extension)
        , m_sourceName(sourceName)
    {}

    ImportExportBufferSource(const Name& extension, const QString& sourceName)
        : m_extension(extension)
        , m_sourceName(sourceName)
    {}

    void SetData(const QByteArray& data)
    {
        m_buffer.setData(data);
    }

    QIODevice* GetDevice() override { return &m_buffer; }
    const Name& GetExtension() const override { return m_extension; }
    const QString& GetSourceName() const override { return m_sourceName; }

private:
    QBuffer m_buffer;
    Name m_extension;
    QString m_sourceName;
};

class ImportExportFileSource : public ImportExportSource
{
public:
    ImportExportFileSource(const QUrl& filePath)
        : m_sourceName(filePath.toLocalFile())
        , m_file(m_sourceName)
    {
        QFileInfo fileInfo(m_sourceName);
        m_extension = Name(fileInfo.completeSuffix());
    }

    QIODevice* GetDevice() override { return &m_file; }
    const Name& GetExtension() const override { return m_extension; }
    const QString& GetSourceName() const override { return m_sourceName; }

    static SharedPointer<ImportExportFileSource> FromFilePath(const QString& filePath)
    {
        return ::make_shared<ImportExportFileSource>(QUrl::fromLocalFile(filePath));
    }

private:
    QString m_sourceName;
    QFile m_file;
    Name m_extension;    
};

struct ImportExportVersion
{
    qint64 Key;
    qint32 Version;
};

using ImportExportVersionPtr = SharedPointer<ImportExportVersion>;

template<class T>
class ImportExportDelegate
{
    Q_DECLARE_TR_FUNCTIONS(ImportExportDelegate)
public:
    struct GetVersionResult
    {
        AsyncResult Result;
        ImportExportVersionPtr VersionInfo;
    };

    using FImportDelegate = std::function<AsyncResult (const ImportExportSourcePtr&, const T&)>;
    using FExportDelegate = std::function<AsyncResult (const ImportExportSourcePtr&, const T&)>;

    ImportExportDelegate(const FImportDelegate& importDelegate = nullptr, const FExportDelegate& exportDelegate = nullptr)
        : ImportDelegate(importDelegate)
        , ExportDelegate(exportDelegate)
    {}
    virtual ~ImportExportDelegate(){}

    FImportDelegate ImportDelegate;
    FExportDelegate ExportDelegate;

    static ImportExportDelegate<T> CreateSerializerXmlStandardDelegate(const SerializerXmlVersion* version, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerXmlReadBuffer&, const T& obj)>& importer = nullptr, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerXmlWriteBuffer&, const T& obj)>& exporter = nullptr);
    static ImportExportDelegate<T> CreateSerializerStandardDelegate(const SerializerVersion* version, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerReadBuffer&, const T& obj)>& importer = nullptr, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerWriteBuffer&, const T& obj)>& exporter = nullptr);

    static ImportExportDelegate<T> CreateSerializerXmlStandardPtrDelegate(const SerializerXmlVersion* version)
    {
        return CreateSerializerXmlStandardDelegate(version, [](const ImportExportSourcePtr&, SerializerXmlReadBuffer& buffer, const T& obj) -> qint8 {
            buffer << *obj;
            return true;
        }, [](const ImportExportSourcePtr&, SerializerXmlWriteBuffer& buffer, const T& obj) -> qint8 {
            buffer << *obj;
            return true;
        });
    }
    static ImportExportDelegate<T> CreateSerializerStandardPtrDelegate(const SerializerVersion* version)
    {
        return CreateSerializerStandardDelegate(version, [](const ImportExportSourcePtr&, SerializerReadBuffer& buffer, const T& obj) -> qint8 {
            buffer << *obj;
            return true;
        }, [](const ImportExportSourcePtr&, SerializerWriteBuffer& buffer, const T& obj) -> qint8 {
            buffer << *obj;
            return true;
        });
    }
};

class ImportExportFilterExtensionsBuilder
{
public:
    ImportExportFilterExtensionsBuilder(bool addZip = false);

    ImportExportFilterExtensionsBuilder& AddAllTypes(bool enable = true);

    ImportExportFilterExtensionsBuilder& AddExtension(const QString& extension);
    ImportExportFilterExtensionsBuilder& AddExtensions(const QStringList& extensions);
    ImportExportFilterExtensionsBuilder& AddExtensions(const QSet<Name>& extensions);
    ImportExportFilterExtensionsBuilder& AddExtension(const QString& label, const QString& extension);

    QStringList Result();

private:
    using ExtensionValue = std::pair<QString, QString>;
    QVector<ExtensionValue> m_extensions;
    std::function<void (QStringList&)> m_addAllTypes;
    std::function<void (const ExtensionValue&)> m_addZip;
};

template<class T>
class ImportExportFactory : public QHash<Name, ImportExportDelegate<T>>
{
    using Super = QHash<Name, ImportExportDelegate<T>>;
public:
    using Super::Super;

    void Add(const Name& ext, const ImportExportDelegate<T>& delegate)
    {
        if(Super::isEmpty()) {
            m_defaultSuffix = ext;
        }
        Super::insert(ext, delegate);
        if(delegate.ImportDelegate != nullptr) {
            m_importExtensions.insert(ext);
        }
        if(delegate.ExportDelegate != nullptr) {
            m_exportExtensions.insert(ext);
        }
    }

    const Name& GetDefaultExportSuffix() const { return m_defaultSuffix; }

    QList<QString> ImportExtensionsFilterList() const
    {
        return ImportExportFilterExtensionsBuilder().AddExtensions(m_importExtensions).AddAllTypes().Result();
    }

    QList<QString> ExportExtensionsFilterList() const
    {
        return ImportExportFilterExtensionsBuilder().AddExtensions(m_exportExtensions).Result();
    }

    const QSet<Name>& GetImportExtensions() const { return m_importExtensions; }
    const QSet<Name>& GetExportExtensions() const { return m_exportExtensions; }

private:
    QSet<Name> m_importExtensions;
    QSet<Name> m_exportExtensions;
    Name m_defaultSuffix;
};

template<class T>
struct DescImportExportTableImport
{
    enum DefaultTextType
    {
        TextType
    };

    using FLoader = std::function<QList<T> (const ImportExportSourcePtr& source)>;
    using FConverter = std::function<QList<QVector<QVariant>> (const QList<T>&)>;
    FLoader Loader;
    FConverter Converter;

    DescImportExportTableImport(const FLoader& loader, const FConverter& converter = [](const QList<T>& value) { return value; })
        : Loader(loader)
        , Converter(converter)
    {}

    DescImportExportTableImport(DefaultTextType type)
        : Loader([](const ImportExportSourcePtr& source){
                QTextStream stream(source->GetDevice());
                QList<QString> data;
                while(!stream.atEnd()) {
                    data.append(stream.readLine());
                }
                return data;
            })
        , Converter([](const QList<QString>& data){
                QList<QVector<QVariant>> result;
                for(const auto& row : data) {
                    QVector<QVariant> rowVariants;
                    for(const auto& stringValue : row.split(',')) {
                        rowVariants.append(stringValue);
                    }
                    result.append(rowVariants);
                }

                return result;
            })
    {}
};

template<class T>
struct DescImportExportTableExport
{
    using ExportDataValueType = QVariant;
    using RowType = QVector<ExportDataValueType>;

    using FSaver = std::function<bool (const ImportExportSourcePtr& source, const QList<QVector<ExportDataValueType>>& data)>;
    using FExtractor = std::function<QList<RowType> (const ImportExportSourcePtr& source, const T& value, const DescImportExportTableExport& params)>;
    FSaver Saver;
    FExtractor Extractor;

    DescImportExportTableExport(const FSaver& saver, const FExtractor& extractor, qint32 role)
        : Saver(saver)
        , Extractor(extractor)
    {
        if(Extractor == nullptr) {
            Extractor = [role](const ImportExportSourcePtr& source, const T& model, const DescImportExportTableExport& params){
                QList<RowType> dataToExport;
                int count = model->columnCount();
                if(source->StandardProperties.IsExportHeader) {
                    RowType header;
                    header.reserve(count);
                    for(int i = 0; i < count; ++i){
                        header << model->headerData(i, Qt::Horizontal);
                    }
                    dataToExport << header;
                }
                auto rowCount = model->rowCount() - model->property(WidgetProperties::ExtraFieldsCount).toInt();
                RowType data;
                data.reserve(count);
                for(int r = 0; r<rowCount; ++r){
                    for(int c = 0; c < count; ++c){
                        auto index = model->index(r,c);
                        data << index.data(role);
                    }
                    dataToExport << data;
                    data.clear();
                }
                return dataToExport;
            };
        }
    }

    DescImportExportTableExport(const QChar& separator)
        : DescImportExportTableExport([separator](const ImportExportSourcePtr& source, const QList<RowType>& data) -> bool {
                QTextStream stream(source->GetDevice());
                StringBuilder::Join(stream, '\n', data, [&](const auto& it){
                    StringBuilder::Join(stream, separator, *it, [&](const auto& it){
                        stream << it->toString();
                    });
                });
                return true;
            }, nullptr, Qt::DisplayRole)
    {}
};

class ImportExport
{
    Q_DECLARE_TR_FUNCTIONS(ImportExport)
    ImportExport(){}
public:
    static SerializerXmlVersion ReadVersionXML(QIODevice* device, const QChar& separator = ';') {
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

    static SerializerVersion ReadVersion(QIODevice* device) {
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

    template<class T>
    static AsyncResult ImportTable(const ImportExportSourcePtr& source, class QAbstractItemModel* model, const DescImportExportTableImport<T>& params)
    {
        return StandardImportExportDevice(source, QIODevice::ReadOnly, [=]() -> qint8 {
            auto data = params.Loader(source);
            if(data.isEmpty()) {
                return false;
            }
            if(source->StandardProperties.IsAutoMatching) {
                auto convertedData = params.Converter(data);
                auto targetCount = model->rowCount();
                auto sourceCount = convertedData.size();
                targetCount -= model->property(WidgetProperties::ExtraFieldsCount).toInt();
                if(sourceCount < targetCount){
                    model->removeRows(sourceCount, targetCount - sourceCount);
                } else if(targetCount < sourceCount){
                    model->insertRows(targetCount, sourceCount - targetCount);
                }
                for(int r = 0; r < convertedData.size(); ++r){
                    const auto& rowData = convertedData[r];
                    for(int c = 0; c < model->columnCount(); ++c){
                        const auto& value = rowData.size() > c ? rowData[c] : QVariant();
                        model->setData(model->index(r, c), value, Qt::EditRole);
                    }
                }
                return true;
            }

            auto hasOverride = applyImportHandler(source, data);
            if(hasOverride.first) {
                return hasOverride.second;
            }

            FutureResult future;
            bool result = false;
            future += ThreadHandlerMain([&result, &data, model, &source]{
                WidgetsImportTableDialog dialog;
                dialog.GetView()->Initialize(data, model, {});
                dialog.exec();
                result = dialog.result() != WidgetsImportTableDialog::IR_Canceled;
                source->Properties.SetProperty("Mode", dialog.result());
            });
            future.Wait();
            return result;
        });
    }

    static AsyncResult StandardImportExportDevice(const ImportExportSourcePtr& source, QIODevice::OpenMode mode, bool multithread, const std::function<qint8 ()>& handler)
    {
        AsyncResult result;

        auto tryOpen = [source, handler, mode]() -> qint8 {
            try {
                if(!source->GetDevice()->open(mode)) {
                    source->SetError(tr("Device cannot be open(%1)").arg(source->GetDevice()->errorString()));
                    return false;
                }
                return handler();
            } catch (...) {
                source->SetError(tr("Undefined error"));
                return false;
            }
        };

        if(multithread) {
            async([result, source, handler, tryOpen]{
                result.Resolve(tryOpen);
            });
        } else {
            result.Resolve(tryOpen);
        }
        return result;
    }

    static AsyncResult StandardImportExportDevice(const ImportExportSourcePtr& source, QIODevice::OpenMode mode, const std::function<qint8 ()>& handler)
    {
        return StandardImportExportDevice(source, mode, source->StandardProperties.IsMultithread, handler);
    }

    static AsyncResult StandardSerializerXmlExportDevice(const ImportExportSourcePtr& source, const SerializerXmlVersion* version, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerXmlWriteBuffer& buffer)>& handler)
    {
        return StandardImportExportDevice(source, QIODevice::WriteOnly, [=]() -> qint8{
            QXmlStreamWriter writer(source->GetDevice());
            writer.setAutoFormatting(!source->StandardProperties.IsCompressed);
            SerializerXmlWriteBuffer buffer(&writer);
            buffer.WriteVersion(*version);
            return handler(source, buffer);
        });
    }

    static AsyncResult StandardSerializerExportDevice(const ImportExportSourcePtr& source, const SerializerVersion* version, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerWriteBuffer& buffer)>& handler)
    {
        return StandardImportExportDevice(source, QIODevice::WriteOnly, [=]() -> qint8{
            SerializerWriteBuffer buffer(source->GetDevice());
            auto hasher = buffer.WriteVersion(*version);
            auto result = handler(source, buffer);
            if(!result) {
                return false;
            }
            hasher.WriteHashSum(source->GetDevice()->size());
            return true;
        });
    }

    static AsyncResult StandardSerializerXmlImportDevice(const ImportExportSourcePtr& source, const SerializerXmlVersion* version, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerXmlReadBuffer& buffer)>& handler)
    {
        return StandardImportExportDevice(source, QIODevice::ReadOnly, [=]() -> qint8{
            QXmlStreamReader reader(source->GetDevice());
            SerializerXmlReadBuffer buffer(&reader);
            auto currentVersion = buffer.ReadVersion();
            auto result = version->CheckVersion(currentVersion);
            source->SetError(result);
            if(result.isValid()) {
                return false;
            }
            return handler(source, buffer);
        });
    }

    static AsyncResult StandardSerializerImportDevice(const ImportExportSourcePtr& source, const SerializerVersion* version, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerReadBuffer& buffer)>& handler)
    {
        return StandardImportExportDevice(source, QIODevice::ReadOnly, [=]() -> qint8{
            SerializerReadBuffer buffer(source->GetDevice());
            auto currentVersion = buffer.ReadVersion();
            auto result = version->CheckVersion(currentVersion, buffer.GetDevice()->size());
            source->SetError(result);
            if(result.isValid()) {
                return false;
            }
            return handler(source, buffer);
        });
    }

    template<class T>
    static AsyncResult ExportTable(const ImportExportSourcePtr& source, class QAbstractItemModel* model, const DescImportExportTableExport<T>& params)
    {
        auto dataToExport = params.Extractor(source, model, params);

        return StandardImportExportDevice(source, QIODevice::WriteOnly, [=]()-> qint8 {
            return params.Saver(source, dataToExport);
        });
    }

    template<class T>
    static AsyncResult Import(const T& target, const ImportExportFactory<T>& factory, const QList<ImportExportSourcePtr>& sources)
    {
        // Note: Copying of delegate is not a mistake
        return importExport<T>(sources, factory, [&target](const ImportExportSourcePtr& source, ImportExportDelegate<T> delegate) -> AsyncResult {
            if(delegate.ImportDelegate == nullptr) {
                return AsyncError();
            }
            return delegate.ImportDelegate(source, target);
        });
    }

    template<class T>
    static AsyncResult Export(const T& target, const ImportExportFactory<T>& factory, const QList<ImportExportSourcePtr>& sources)
    {
        // Note: Copying of delegate is not a mistake
        return importExport<T>(sources, factory, [&target](const ImportExportSourcePtr& source, ImportExportDelegate<T> delegate) -> AsyncResult {
            if(delegate.ExportDelegate == nullptr) {
                return AsyncError();
            }
            return delegate.ExportDelegate(source, target);
        });
    }

    static QString StandardImportingString();
    static QString StandardExportingString();

private:
    friend class ThreadsBase;

    template<class T>
    static std::pair<bool, bool> applyImportHandler(const ImportExportSourcePtr& source, const QList<T>& data);

    template<class T>
    static AsyncResult importExport(const QList<ImportExportSourcePtr>& sources, const ImportExportFactory<T>& factory, const std::function<AsyncResult (const ImportExportSourcePtr& source, const ImportExportDelegate<T>& delegate)>& delegateCall)
    {
        FutureResult future;
        for(const auto& source : sources) {
            auto foundIt = factory.find(source->GetExtension());
            if(foundIt != factory.end()) {
                future += delegateCall(source, foundIt.value());
            } else {
                source->SetError(tr("%1 extension is not supported").arg(source->GetExtension().AsString()));
            }
        }
        return future.ToAsyncResult();
    }

    static AsyncResult async(const FAction& task, EPriority priority = EPriority::Low);
    static ThreadPool& threadPool();
};

template<>
inline std::pair<bool, bool> ImportExport::applyImportHandler<QString>(const ImportExportSourcePtr& source, const QList<QString>& data)
{
    const auto& handler = source->StandardProperties.ImportTableStringHandler;
    if(handler == nullptr) {
        return std::make_pair(false, false);
    }
    return std::make_pair(true, handler(data));
}

template<>
inline std::pair<bool, bool> ImportExport::applyImportHandler<QVector<QVariant>>(const ImportExportSourcePtr& source, const QList<QVector<QVariant>>& data)
{
    const auto& handler = source->StandardProperties.ImportTableDataHandler;
    if(handler == nullptr) {
        return std::make_pair(false, false);
    }
    return std::make_pair(true, handler(data));
}

template<class T>
inline ImportExportDelegate<T> ImportExportDelegate<T>::CreateSerializerXmlStandardDelegate(const SerializerXmlVersion* version, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerXmlReadBuffer&, const T& obj)>& importer, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerXmlWriteBuffer&, const T& obj)>& exporter)
{
    ImportExportDelegate<T> result;
    if(importer != nullptr) {
        result.ImportDelegate = [=](const ImportExportSourcePtr& source, const T& obj) -> AsyncResult {
            return ImportExport::StandardSerializerXmlImportDevice(source, version, [=](const ImportExportSourcePtr& source, SerializerXmlReadBuffer& buffer)-> qint8 {
                return importer(source, buffer, obj);
            });
        };
    }

    if(exporter != nullptr) {
        result.ExportDelegate = [=](const ImportExportSourcePtr& source, const T& obj) -> AsyncResult {
            return ImportExport::StandardSerializerXmlExportDevice(source, version, [=](const ImportExportSourcePtr& source, SerializerXmlWriteBuffer& buffer)-> qint8 {
                return exporter(source, buffer, obj);
            });
        };
    }
    return result;
}

template<class T>
inline ImportExportDelegate<T> ImportExportDelegate<T>::CreateSerializerStandardDelegate(const SerializerVersion* version, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerReadBuffer&, const T& obj)>& importer, const std::function<qint8 (const ImportExportSourcePtr& source, SerializerWriteBuffer&, const T& obj)>& exporter)
{
    ImportExportDelegate<T> result;
    if(importer != nullptr) {
        result.ImportDelegate = [=](const ImportExportSourcePtr& source, const T& obj) -> AsyncResult {
            return ImportExport::StandardSerializerImportDevice(source, version, [=](const ImportExportSourcePtr& source, SerializerReadBuffer& buffer)-> qint8 {
                return importer(source, buffer, obj);
            });
        };
    }

    if(exporter != nullptr) {
        result.ExportDelegate = [=](const ImportExportSourcePtr& source, const T& obj) -> AsyncResult {
            return ImportExport::StandardSerializerExportDevice(source, version, [=](const ImportExportSourcePtr& source, SerializerWriteBuffer& buffer)-> qint8 {
                return exporter(source, buffer, obj);
            });
        };
    }
    return result;
}

class ImportExportFormatFactory
{
    Q_DECLARE_TR_FUNCTIONS(ImportExportFormatFactory)
public:
    using FFormatExtractorDelegate = std::function<bool (const ImportExportSourcePtr& source)>;
    using FormatDelegates = QHash<Name, FFormatExtractorDelegate>;
    using FHandler = std::function<AsyncResult (const ImportExportSourcePtr& source)>;
    using Container = QVector<std::pair<FFormatExtractorDelegate, FHandler>>;

    void Register(const FormatDelegates& delegates, const FHandler& handler);
    void Register(const QSet<Name>& extensions, const FHandler& handler);

    AsyncResult Export(const QList<ImportExportSourcePtr>& sources) const
    {
        return Import(sources);
    }
    AsyncResult Import(const QList<ImportExportSourcePtr>& sources) const;

    const QHash<Name, Container>& GetFactory() const { return m_factory; }

private:
    QHash<Name, Container>  m_factory;
};

#endif // ImportExport_H
