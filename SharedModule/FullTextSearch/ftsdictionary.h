#ifndef FTSDICTIONARY_H
#define FTSDICTIONARY_H

#include <SharedModule/internal.hpp>

class FTSDictionary;

class FTSObject
{
public:
    FTSObject(FTSDictionary* dictionary);

    void AddRow(const QString& string, size_t rowId);

private:
    FTSDictionary* m_dictionary;
};

struct FTSObjectRow
{
    FTSObject* Object;
    size_t Id;
    double weight;

    bool operator==(const FTSObjectRow& another) const { return Object == another.Object && Id == another.Id; }
};

struct FTSMatchedObject
{
    FTSObjectRow Row;
    double matchesWeight;

    friend QDebug operator<<(QDebug out, const FTSMatchedObject& object)
    {
        out.space() << "Match:" << object.Row.Object << object.Row.Id << "Count:" << object.matchesWeight;
        return out;
    }
};

class FTSMatchResult : public QVector<FTSMatchedObject>
{
public:
    void Sort();
    void SortAndFilter();
};

class FTSDictionary
{
    friend class FTSObject;
public:
    FTSDictionary();

    FTSMatchResult Match(const QString& string) const;
    QMap<qint32, FTSMatchedObject> Map(const QStringList& strings) const;

private:
    void addRow(FTSObject* object, const QString& string, size_t rowId);
    bool parseString(const QString& string, const std::function<void (const Name&, double)>& onStringPartSplited) const;

private:
    QHash<Name, QSet<FTSObjectRow>> m_dictionary;
    QRegExp m_digitsRegexp;
    QRegExp m_nonDigitsLettersSpacesRegexp;
};

#endif // FTSDICTIONARY_H
