#include "iconsmanager.h"

#include <QDomDocument>
#include <QSvgRenderer>
#include <QIconEngine>

IconsManager* IconsManager::m_instance = nullptr;

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
    LocalPropertyString FilePath;
    IconsPalette Palette;

    void Clear()
    {
        Cache.clear();
        Source.clear();
        IsValid = false;
    }

private:
    struct HashKey
    {
        QIcon::Mode Mode;
        QIcon::State State;

        operator qint64() const { return *(qint64*)this; }
    };

    friend class SvgIconEngine;
    QSvgRenderer Renderer;
    QHash<Size, QHash<HashKey, QPixmap>> Cache;
    QDomDocument Source;
    bool IsValid;
};

class SvgIconEngine : public QIconEngine
{
public:
    SvgIconEngine();
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
    void setAttributeRecursive(QDomElement elem, QString strtagname, QString strattr, QString strattrval) const;
    void generateSource();
    QPixmap generatePixmap(const QSize &size, QIcon::Mode mode,
                           QIcon::State state) const;

private:
    SharedPointer<SvgIconEngineData> d;
    DispatcherConnectionsSafe m_connections;
};

SvgIconEngine::SvgIconEngine()
    : d(new SvgIconEngineData)
{
    d->FilePath.OnChanged.Connect(this, [this]{
        generateSource();
    }).MakeSafe(m_connections);

    auto resetCache = [this](QIcon::Mode mode) {
        for(auto& modes : d->Cache) {
            modes.remove({ mode, QIcon::On });
            modes.remove({ mode, QIcon::Off });
        }
    };

    d->Palette.ActiveColor.OnChanged.Connect(this, [resetCache]{
        resetCache(QIcon::Active);
    }).MakeSafe(m_connections);
    d->Palette.NormalColor.OnChanged.Connect(this, [resetCache]{
        resetCache(QIcon::Normal);
    }).MakeSafe(m_connections);
    d->Palette.SelectedColor.OnChanged.Connect(this, [resetCache]{
        resetCache(QIcon::Selected);
    }).MakeSafe(m_connections);
    d->Palette.DisabledColor.OnChanged.Connect(this, [resetCache]{
        resetCache(QIcon::Disabled);
    }).MakeSafe(m_connections);
}

SvgIconEngine::SvgIconEngine(const SvgIconEngine& another)
    : d(another.d)
{
}

SvgIconEngine::~SvgIconEngine()
{
}

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


void SvgIconEngine::generateSource()
{
    d->Cache.clear();
    if(fileType(QFileInfo(d->FilePath)) == SvgFile) {
        QFile file(d->FilePath);
        if(file.open(QFile::ReadOnly)) {
            auto parseResult = d->Source.setContent(file.readAll());
            d->IsValid = parseResult;
            return;
        }
    }
    d->IsValid = false;
}

QPixmap SvgIconEngine::pixmap(const QSize& size, QIcon::Mode mode,
                               QIcon::State state)
{
    if(!d->IsValid) {
        return QPixmap();
    }

    auto foundIt = d->Cache.find(size);
    if(foundIt != d->Cache.end()) {
        auto foundItPixmap = foundIt.value().find({ mode, state });
        if(foundItPixmap != foundIt.value().end()) {
            return foundItPixmap.value();
        }

        auto pixmap = generatePixmap(size,mode,state);
        foundIt.value().insert({ mode, state }, pixmap);
        return pixmap;
    }

    auto pixmap = generatePixmap(size,mode,state);
    d->Cache.insert(size, { { { mode, state }, pixmap} });
    return pixmap;
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
    if (painter->device())
        pixmapSize *= painter->device()->devicePixelRatioF();
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

void SvgIconEngine::setAttributeRecursive(QDomElement elem, QString strtagname, QString strattr, QString strattrval) const
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

QPixmap SvgIconEngine::generatePixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) const
{
    Q_ASSERT(d->IsValid);

    auto& doc = d->Source;
    // recurivelly change color
    if(state == QIcon::On) {
        setAttributeRecursive(doc.documentElement(), "path", "fill", d->Palette.SelectedColor.Native().name());
    } else {
        switch (mode) {
        case QIcon::Active: setAttributeRecursive(doc.documentElement(), "path", "fill", d->Palette.ActiveColor.Native().name()); break;
        case QIcon::Disabled: setAttributeRecursive(doc.documentElement(), "path", "fill", d->Palette.DisabledColor.Native().name()); break;
        case QIcon::Selected: setAttributeRecursive(doc.documentElement(), "path", "fill", d->Palette.SelectedColor.Native().name()); break;
        default: setAttributeRecursive(doc.documentElement(), "path", "fill", d->Palette.NormalColor.Native().name()); break;
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

IconsSvgIcon::IconsSvgIcon()
    : Super(m_engine = new SvgIconEngine())
{}

IconsSvgIcon::IconsSvgIcon(const QString& filePath)
    : IconsSvgIcon()
{
    m_engine->GetData()->FilePath = filePath;
}

IconsPalette& IconsSvgIcon::EditPalette() const
{
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

    icon.EditPalette().NormalColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, styleSettings.IconPrimaryColor);
    icon.EditPalette().DisabledColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, styleSettings.IconPrimaryColor);
    icon.EditPalette().ActiveColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, styleSettings.IconSelectionColor);
    icon.EditPalette().SelectedColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, styleSettings.IconSelectionColor);

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
