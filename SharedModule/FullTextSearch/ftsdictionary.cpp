#include "ftsdictionary.h"

uint qHash(const FTSObjectRow& row, uint seed=0)
{
    return qHashBits(&row, sizeof(FTSObjectRow), seed);
}

FTSDictionary::FTSDictionary()
    : m_digitsRegexp(R"((\d+))")
    , m_nonDigitsLettersSpacesRegexp(R"([\(\)\[\]\"\"])")
{
}

void FTSDictionary::addRow(FTSObject* object, const QString& string, size_t rowId)
{
    parseString(string, [this, object, rowId](const Name& stringPart){
        m_dictionary[stringPart].insert({object,rowId});
    });
}

void FTSMatchResult::Sort()
{
    std::sort(begin(), end(), [](const FTSMatchedObject& f, const FTSMatchedObject& s){
        return f.matchesCount > s.matchesCount;
    });
}

void FTSMatchResult::SortAndFilter()
{
    if(!isEmpty()) {
        Sort();
        auto bestMatches = first().matchesCount;
        auto newEnd = std::remove_if(begin(), end(), [bestMatches](const FTSMatchedObject& f){
            return bestMatches != f.matchesCount;
        });
        resize(std::distance(begin(), newEnd));
    }
}

FTSMatchResult FTSDictionary::Match(const QString& string) const
{
    FTSMatchResult result;
    QHash<FTSObjectRow, qint32> rows;
    parseString(string, [this, &rows](const Name& stringPart) {
        auto foundItDict = m_dictionary.find(stringPart);
        if(foundItDict != m_dictionary.end()) {
            for(const auto& objectRow : foundItDict.value()) {
                auto foundIt = rows.find(objectRow);
                if(foundIt == rows.end()) {
                    rows.insert(objectRow, 1);
                } else {
                    foundIt.value()++;
                }
            }
        }
    });

    for(auto it(rows.begin()), e(rows.end()); it != e; it++) {
        result.append({it.key(), it.value()});
    }

    return result;
}

bool FTSDictionary::parseString(const QString &string, const std::function<void (const Name&)>& onStringPartSplited) const
{
    if(string.size() < 3) {
        return false;
    }
    auto lowerString = string.toLower();

    qint32 index = 0;
    while((index = m_digitsRegexp.indexIn(lowerString, index)) != -1) {
        onStringPartSplited(Name(m_digitsRegexp.cap(1)));
        index += m_digitsRegexp.matchedLength();
    }

    lowerString.remove(m_nonDigitsLettersSpacesRegexp);
    lowerString += " ";

    auto ii1 = lowerString.begin();
    auto ii2 = lowerString.begin() + 1;
    auto ie = lowerString.end();
    QString cw(" ");
    cw += *ii1;
    do {
        while(ii2 != ie && cw.size() != 3 ){
            cw += *ii2;
            ii2++;
        }
        if(cw.size() != 3) {
            break;
        }
        onStringPartSplited(Name(cw));
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
