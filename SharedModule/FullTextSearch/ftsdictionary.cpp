#include "ftsdictionary.h"

uint qHash(const FTSObjectRow& row, uint seed=0)
{
    return qHashBits(&row, sizeof(FTSObjectRow) - sizeof(double), seed);
}

FTSDictionary::FTSDictionary()
    : m_digitsRegexp(R"((\d+))")
    , m_nonDigitsLettersSpacesRegexp(R"([\(\)\[\]\"\"])")
{
}

void FTSDictionary::addRow(FTSObject* object, const QString& string, size_t rowId)
{
    parseString(string, [this, object, rowId](const Name& stringPart, double weight){
        m_dictionary[stringPart].insert({object,rowId,weight});
    });
}

void FTSMatchResult::Sort()
{
    std::sort(begin(), end(), [](const FTSMatchedObject& f, const FTSMatchedObject& s){
        return f.matchesWeight > s.matchesWeight;
    });
}

void FTSMatchResult::SortAndFilter()
{
    if(!isEmpty()) {
        Sort();
        auto bestMatches = first().matchesWeight;
        auto newEnd = std::remove_if(begin(), end(), [bestMatches](const FTSMatchedObject& f){
            return (bestMatches - f.matchesWeight) > 0.5f;
        });
        resize(std::distance(begin(), newEnd));
    }
}

FTSMatchResult FTSDictionary::Match(const QString& string) const
{
    FTSMatchResult result;
    QHash<FTSObjectRow, double> rows;
    parseString(string, [this, &rows](const Name& stringPart, double weight) {
        auto rowView = rows;
        auto foundItDict = m_dictionary.find(stringPart);
        if(foundItDict != m_dictionary.end()) {
            for(const auto& objectRow : foundItDict.value()) {
                auto foundIt = rows.find(objectRow);
                if(foundIt == rows.end()) {
                    rows.insert(objectRow, qMin(objectRow.weight, weight));
                } else {
                    foundIt.value() += qMin(objectRow.weight, weight);
                }
            }
        }
    });

    for(auto it(rows.begin()), e(rows.end()); it != e; it++) {
        result.append({it.key(), it.value()});
    }

    return result;
}

QMap<qint32, FTSMatchedObject> FTSDictionary::Map(const QStringList& strings) const
{
    QMap<qint32, FTSMatchedObject> bestMapping;
    QVector<FTSMatchResult> temp;
    for(const auto& string : strings) {
        auto res = Match(string);
        res.Sort();
        temp.append(res);
    }

    QHash<FTSObjectRow, qint32> objectsUse;

    QVector<qint32> indices;
    indices.resize(temp.size());
    std::iota(indices.begin(), indices.end(), 0);

    while(!indices.isEmpty()) {
        QVector<qint32> newIndices;
        for(qint32 row : indices) {
            const auto& tempResult = temp[row];
            bestMapping[row] = { {nullptr, 0, 0.0}, 0.0 };
            for(const auto& match : tempResult) {
                auto foundIt = objectsUse.find(match.Row);
                if(foundIt != objectsUse.end()) {
                    auto oldIndex = foundIt.value();
                    auto currentMatch = bestMapping[oldIndex].matchesWeight;
                    if(currentMatch < match.matchesWeight) {
                        objectsUse.erase(foundIt);
                        bestMapping[row] = match;
                        objectsUse[match.Row] = row;
                        newIndices.append(oldIndex);
                        break;
                    }
                } else {
                    objectsUse.insert(match.Row, row);
                    bestMapping[row] = match;
                    break;
                }
            }
        }
        indices = newIndices;
    }

    return bestMapping;
}

bool FTSDictionary::parseString(const QString &string, const std::function<void (const Name&, double weight)>& onStringPartSplited) const
{
    if(string.size() < 2) {
        return false;
    }
    auto lowerString = string.toLower();

    qint32 index = 0;
    while((index = m_digitsRegexp.indexIn(lowerString, index)) != -1) {
        onStringPartSplited(Name(m_digitsRegexp.cap(1)), 1.f + float(m_digitsRegexp.cap(1).length()) / string.size());
        index += m_digitsRegexp.matchedLength();
    }

    lowerString.remove(m_nonDigitsLettersSpacesRegexp);
    lowerString += " ";

    auto ii1 = lowerString.begin();
    auto ii2 = lowerString.begin() + 1;
    auto ie = lowerString.end();
    QString cw(" ");
    cw += *ii1;
    auto tripletWeight = 1.f / (lowerString.size() - 1);
    do {
        while(ii2 != ie && cw.size() != 3 ){
            cw += *ii2;
            ii2++;
        }
        if(cw.size() != 3) {
            break;
        }
        onStringPartSplited(Name(cw), tripletWeight);
        cw = QString(*ii1);
        ii2 = ii1 + 1;
        ii1++;
    } while(ii2 != ie);
    return true;
}

FTSObject::FTSObject(FTSDictionary* dictionary)
    : m_dictionary(dictionary)
{

}


void FTSObject::AddRow(const QString& string, size_t rowId)
{
    m_dictionary->addRow(this, string, rowId);
}
