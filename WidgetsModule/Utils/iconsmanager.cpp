#include "iconsmanager.h"

#include <QDomDocument>
#include <QSvgRenderer>
#include <QIconEngine>

IconsManager* IconsManager::m_instance = nullptr;


enum FileType { OtherFile, SvgFile, CompressedSvgFile };
static FileType fileType(const QFileInfo& fi)
{
    const QString &abs = fi.absoluteFilePath();
    if (abs.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive))
        return SvgFile;
    if (abs.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)
        || abs.endsWith(QLatin1String(".svg.gz"), Qt::CaseInsensitive)) {
        return CompressedSvgFile;
    }
    return OtherFile;
}

class SvgIconEngineData
{
    struct Size : public QSize
    {
        using Super = QSize;
    public:
        using Super::Super;
        Size(const QSize& size)
            : Super(size)
        {

        }

        operator qint64() const { return *(qint64*)this; }
    };

public:
    using FPreload = std::function<void (const IconsPalette&, QByteArray&)>;
    SvgIconEngineData(const FPreload& preload = nullptr)
        : Preload(preload)
    {
        FilePath.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
            generateSource();
        }).MakeSafe(m_connections);

        auto resetCache = [this](QIcon::Mode ) {
            m_clearCacheDelayed.Call(CONNECTION_DEBUG_LOCATION, [this]{
                if(Preload == nullptr) {
                    Cache.clear();
                } else {
                    if(Renderer == nullptr) {
                        Renderer = new QSvgRenderer();
                    }
                    auto ba = Source.toByteArray();
                    Preload(Palette, ba);
                    Renderer->load(ba);
                    if(Renderer->animated()) {
                        Renderer->connect(Renderer.get(), &QSvgRenderer::repaintNeeded, [this]{
                            Cache.clear();
                        });
                    }
                }
            });
            // TODO. Not working
            /*SvgIconEngineData::HashKey modeOn { mode, QIcon::On };
            SvgIconEngineData::HashKey modeOff { mode, QIcon::Off };
            for(auto& modes : d->Cache) {
                modes.remove(modeOn);
                modes.remove(modeOff);
            }*/
        };

        Palette.ActiveColor.ConnectAction(CONNECTION_DEBUG_LOCATION, [resetCache]{
            resetCache(QIcon::Active);
        }).MakeSafe(m_connections);
        Palette.NormalColor.ConnectAction(CONNECTION_DEBUG_LOCATION, [resetCache]{
            resetCache(QIcon::Normal);
        }).MakeSafe(m_connections);
        Palette.SelectedColor.ConnectAction(CONNECTION_DEBUG_LOCATION, [resetCache]{
            resetCache(QIcon::Selected);
        }).MakeSafe(m_connections);
        Palette.DisabledColor.ConnectAction(CONNECTION_DEBUG_LOCATION, [resetCache]{
            resetCache(QIcon::Disabled);
        }).MakeSafe(m_connections);
    }

    QPixmap Pixmap(const QSize& size, QIcon::Mode mode,
                               QIcon::State state);

    LocalPropertyString FilePath;
    IconsPalette Palette;
    mutable ScopedPointer<QSvgRenderer> Renderer;
    FPreload Preload;
    bool IsValid;

    void Clear()
    {
        Cache.clear();
        Source.clear();
        IsValid = false;
    }

private:
    void setAttributeRecursive(QDomElement elem, QString strtagname, QString strattr, QString strattrval) const;
    QPixmap generatePixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) const;
    void generateSource();

private:
    struct HashKey
    {
        QIcon::Mode Mode;
        QIcon::State State;

        operator qint64() const { return *(qint64*)this; }
    };

    QHash<Size, QHash<HashKey, QPixmap>> Cache;
    QDomDocument Source;

private:
    DispatcherConnectionsSafe m_connections;
    DelayedCallObject m_clearCacheDelayed;
};

void SvgIconEngineData::setAttributeRecursive(QDomElement elem, QString strtagname, QString strattr, QString strattrval) const
{
    // if it has the tagname then overwritte desired attribute
    if (elem.tagName().compare(strtagname) == 0)
    {
        elem.setAttribute(strattr, strattrval);
    }
    // loop all children
    for (int i = 0; i < elem.childNodes().count(); i++)
    {
        if (!elem.childNodes().at(i).isElement())
        {
            continue;
        }
        setAttributeRecursive(elem.childNodes().at(i).toElement(), strtagname, strattr, strattrval);
    }
}

QPixmap SvgIconEngineData::Pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
    if(!IsValid) {
        return QPixmap();
    }
    auto foundIt = Cache.find(size);
    if(foundIt != Cache.end()) {
        auto foundItPixmap = foundIt.value().find({ mode, state });
        if(foundItPixmap != foundIt.value().end()) {
            return foundItPixmap.value();
        }

        auto pixmap = generatePixmap(size,mode,state);
        foundIt.value().insert({ mode, state }, pixmap);
        return pixmap;
    }
    auto pixmap = generatePixmap(size,mode,state);
    Cache.insert(size, { { { mode, state }, pixmap} });
    return pixmap;
}

QPixmap SvgIconEngineData::generatePixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) const
{
    Q_ASSERT(IsValid);

    if(Renderer != nullptr) {
        QPixmap pix(size);
        pix.fill(Qt::transparent);
        QPainter pixPainter(&pix);
        Renderer->render(&pixPainter);
        return pix;
    }

    auto& doc = Source;
    // recurivelly change color
    if(state == QIcon::On) {
        if(Palette.SelectedColor.IsValid) {
            setAttributeRecursive(doc.documentElement(), "path", "fill", Palette.SelectedColor.Value.Native().name());
            setAttributeRecursive(doc.documentElement(), "rect", "stroke", Palette.SelectedColor.Value.Native().name());
        }
    } else {
        switch (mode) {
        case QIcon::Active:
            if(Palette.NormalColor.IsValid) {
                setAttributeRecursive(doc.documentElement(), "path", "fill", Palette.NormalColor.Value.Native().name());
                setAttributeRecursive(doc.documentElement(), "rect", "stroke", Palette.NormalColor.Value.Native().name());
            }
            break;
        case QIcon::Disabled:
            if(Palette.DisabledColor.IsValid) {
                setAttributeRecursive(doc.documentElement(), "path", "fill", Palette.DisabledColor.Value.Native().name());
                setAttributeRecursive(doc.documentElement(), "rect", "stroke", Palette.DisabledColor.Value.Native().name());
            }
            break;
        case QIcon::Selected:
            if(Palette.NormalColor.IsValid) {
                setAttributeRecursive(doc.documentElement(), "path", "fill", Palette.NormalColor.Value.Native().name());
                setAttributeRecursive(doc.documentElement(), "rect", "stroke", Palette.NormalColor.Value.Native().name());
            }
            break;
        default:
            if(Palette.NormalColor.IsValid) {
                setAttributeRecursive(doc.documentElement(), "path", "fill", Palette.NormalColor.Value.Native().name());
                setAttributeRecursive(doc.documentElement(), "rect", "stroke", Palette.NormalColor.Value.Native().name());
            }
            break;
        }
    }

    // create svg renderer with edited contents
    QSvgRenderer svgRenderer(doc.toByteArray());
    QPixmap pix(size);
    pix.fill(Qt::transparent);
    QPainter pixPainter(&pix);
    svgRenderer.render(&pixPainter);
    return pix;
}

void SvgIconEngineData::generateSource()
{
    Cache.clear();
    if(fileType(QFileInfo(FilePath)) == SvgFile) {
        QFile file(FilePath);
        if(file.open(QFile::ReadOnly)) {
            auto parseResult = Source.setContent(file.readAll());
            IsValid = parseResult;
            return;
        }
    }
    IsValid = false;
}

class SvgIconEngine : public QIconEngine
{
public:
    SvgIconEngine();
    SvgIconEngine(const SvgIconEngineData::FPreload& preload);
    SvgIconEngine(const SvgIconEngine& another);
    ~SvgIconEngine();

    void paint(QPainter *painter, const QRect &rect,
               QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state) override;

    void addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                   QIcon::State state) override;
    void addFile(const QString &fileName, const QSize &size,
                 QIcon::Mode mode, QIcon::State state) override;

    QString key() const override;
    QIconEngine *clone() const override;

    void virtual_hook(int id, void *data) override;

    const SharedPointer<SvgIconEngineData>& GetData() const { return d; }

private:
    SharedPointer<SvgIconEngineData> d;
};

SvgIconEngine::SvgIconEngine(const SvgIconEngineData::FPreload& preload)
    : d(new SvgIconEngineData(preload))
{

}

SvgIconEngine::SvgIconEngine()
    : d(new SvgIconEngineData)
{
}

SvgIconEngine::SvgIconEngine(const SvgIconEngine& another)
    : d(another.d)
{
}

SvgIconEngine::~SvgIconEngine()
{
}

QPixmap SvgIconEngine::pixmap(const QSize& size, QIcon::Mode mode,
                               QIcon::State state)
{
    return d->Pixmap(size, mode, state);
}


void SvgIconEngine::addPixmap(const QPixmap&, QIcon::Mode,
                               QIcon::State)
{
    Q_ASSERT(false);
}

void SvgIconEngine::addFile(const QString&, const QSize&,
                             QIcon::Mode , QIcon::State)
{
    Q_ASSERT(false);
}

void SvgIconEngine::paint(QPainter* painter, const QRect& rect,
                           QIcon::Mode mode, QIcon::State state)
{
    QSize pixmapSize = rect.size();
    if (painter->device()) {
        pixmapSize *= painter->device()->devicePixelRatioF();
    }
    painter->drawPixmap(rect, pixmap(pixmapSize, mode, state));
}

QString SvgIconEngine::key() const
{
    return QLatin1String("svg");
}

QIconEngine* SvgIconEngine::clone() const
{
    return new SvgIconEngine(*this);
}

void SvgIconEngine::virtual_hook(int id, void* data)
{
    if (id == QIconEngine::IsNullHook) {
        *reinterpret_cast<bool*>(data) = !d->IsValid;
    }
    QIconEngine::virtual_hook(id, data);
}

IconsSvgIcon::IconsSvgIcon()
    : m_engine(nullptr)
{}

IconsSvgIcon::IconsSvgIcon(const QString& filePath)
    : Super(m_engine = new SvgIconEngine())
{
    m_engine->GetData()->FilePath = filePath;
}

IconsSvgIcon::IconsSvgIcon(const QString& filePath, const std::function<void(const IconsPalette&, QByteArray&)>& preload)
    : Super(m_engine = new SvgIconEngine(preload))
{
    m_engine->GetData()->FilePath = filePath;
}

QIcon IconsSvgIcon::MergedIcon(const QSize& size, const QVector<QIcon>& icons, QIcon::Mode mode, QIcon::State state) const
{
    QIcon result;
    QPixmap newPixmap(size.width() * (icons.size() + 1), size.height());

    auto addPixmap = [&](QIcon::Mode mode, QIcon::State state) {
        newPixmap.fill(Qt::transparent);
        QPainter painter(&newPixmap);
        painter.drawPixmap(0,0, pixmap(size, mode, state));
        qint32 step = size.width();
        for(const auto& icon : icons) {
            painter.drawPixmap(step,0, icon.pixmap(size, mode, state));
            step += size.width();
        }
        result.addPixmap(newPixmap, mode, state);
    };

    addPixmap(mode, state);

    return result;
}

QIcon IconsSvgIcon::MergedIcon(const QSize& size, const QVector<QIcon>& icons) const
{
    QIcon result;
    QPixmap newPixmap(size);

    auto addPixmap = [&](QIcon::Mode mode, QIcon::State state) {
        newPixmap.fill(Qt::transparent);
        QPainter painter(&newPixmap);
        painter.drawPixmap(0,0, pixmap(size, mode, state));
        qint32 step = size.width();
        for(const auto& icon : icons) {
            painter.drawPixmap(step,0, icon.pixmap(size, mode, state));
            step += size.width();
        }
        result.addPixmap(newPixmap, mode, state);
    };

    addPixmap(QIcon::Active, QIcon::On);
    addPixmap(QIcon::Normal, QIcon::On);
    addPixmap(QIcon::Disabled, QIcon::On);
    addPixmap(QIcon::Selected, QIcon::On);

    addPixmap(QIcon::Active, QIcon::Off);
    addPixmap(QIcon::Normal, QIcon::Off);
    addPixmap(QIcon::Disabled, QIcon::Off);
    addPixmap(QIcon::Selected, QIcon::Off);

    return result;
}

QSvgRenderer* IconsSvgIcon::GetRenderer() const
{
    if(m_engine == nullptr) {
        return nullptr;
    }
    return m_engine->GetData()->Renderer.get();
}

IconsPalette& IconsSvgIcon::EditPalette() const
{
    Q_ASSERT(m_engine != nullptr);
    static IconsPalette defaultPalette;
    if(m_engine == nullptr) {
        return defaultPalette;
    }
    return m_engine->GetData()->Palette;
}

IconsManager::IconsManager(qint32 iconsCount)
    : m_icons(iconsCount)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;
}

IconsSvgIcon IconsManager::RegisterIcon(qint32 index, const QString& path)
{
    IconsSvgIcon result(path);
    m_icons[index] = result;
    return result;
}

IconsSvgIcon IconsManager::RegisterIcon(const Name& id, const QString& path)
{
    Q_ASSERT(!m_taggedIcons.contains(id));
    IconsSvgIcon result(path);
    m_taggedIcons.insert(id, result);
    return result;
}

IconsSvgIcon IconsManager::RegisterIconWithDefaultColorScheme(const Name& id, const QString& path)
{
    auto icon = RegisterIcon(id, path);

    auto& styleSettings = SharedSettings::GetInstance().StyleSettings;

    icon.EditPalette().NormalColor.ConnectFrom(CDL, styleSettings.IconPrimaryColor);
    icon.EditPalette().DisabledColor.ConnectFrom(CDL, styleSettings.IconDisabledColor);
    icon.EditPalette().ActiveColor.ConnectFrom(CDL, styleSettings.IconPrimaryColor);
    icon.EditPalette().SelectedColor.ConnectFrom(CDL, styleSettings.IconSelectionColor);

    return icon;
}

IconsSvgIcon IconsManager::RegisterIconWithSecondaryColorScheme(const Name& id, const QString& path)
{
    auto icon = RegisterIcon(id, path);

    auto& styleSettings = SharedSettings::GetInstance().StyleSettings;

    icon.EditPalette().NormalColor.ConnectFrom(CDL, styleSettings.IconSecondaryColor);
    icon.EditPalette().DisabledColor.ConnectFrom(CDL, styleSettings.IconSecondaryDisabledColor);
    icon.EditPalette().ActiveColor.ConnectFrom(CDL, styleSettings.IconSecondarySelectionColor);
    icon.EditPalette().SelectedColor.ConnectFrom(CDL, styleSettings.IconSecondarySelectionColor);

    return icon;
}

const IconsSvgIcon& IconsManager::GetIcon(qint32 index) const
{
    return m_icons.at(index);
}

const IconsSvgIcon& IconsManager::GetIcon(const Name& id) const
{
    static IconsSvgIcon defaultIcon;
    auto foundIt = m_taggedIcons.find(id);
    if(foundIt != m_taggedIcons.end()) {
        return foundIt.value();
    }
    return defaultIcon;
}

IconsManager& IconsManager::GetInstance()
{
    return *m_instance;
}

DispatcherConnections IconsPalette::ConnectFrom(const char* location, IconsPalette& another)
{
    DispatcherConnections result;
    result += NormalColor.ConnectFrom(location, another.NormalColor);
    result += DisabledColor.ConnectFrom(location, another.DisabledColor);
    result += ActiveColor.ConnectFrom(location, another.ActiveColor);
    result += SelectedColor.ConnectFrom(location, another.SelectedColor);
    return result;
}

DispatcherConnections IconsPalette::ConnectFrom(const char* location, const LocalPropertyColor& color)
{
    DispatcherConnections result;
    result += NormalColor.ConnectFrom(location, color);
    result += DisabledColor.ConnectFrom(location, color);
    result += ActiveColor.ConnectFrom(location, color);
    result += SelectedColor.ConnectFrom(location, color);
    return result;
}

DispatcherConnections IconsPalette::ConnectFrom(const char* location, const LocalPropertyColor& color, const LocalPropertyColor& checked)
{
    DispatcherConnections result;
    result += NormalColor.ConnectFrom(location, color);
    result += DisabledColor.ConnectFrom(location, color);
    result += ActiveColor.ConnectFrom(location, color);
    result += SelectedColor.ConnectFrom(location, checked);
    return result;
}
