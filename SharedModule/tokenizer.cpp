#include "tokenizer.h"
#include "builders.h"

Tokenizer::Tokenizer(const QString& pattern)
    : m_pattern(pattern)
{

}

Tokenizer::Tokens Tokenizer::CreateTokens(const ProcessFactory& factory)
{
    if(!m_tokens.isEmpty()) {
        return tokenInfoFromFactory(factory);
    }

    auto regExpPattern = StringBuilder("(").Join('|', factory, [](const auto& iterator){
        return R"(\[)" + iterator.key().AsString() + R"(\])";
    }).append(')');

    QRegularExpression regExp(regExpPattern);

    auto it = regExp.globalMatch(m_pattern);
    while(it.hasNext()) {
        auto n = it.next();
        auto keyWord = n.captured(1);
        TokenInfo token;
        token.TokenName = Name(keyWord.mid(1, keyWord.size() - 2));
        token.StartsAt = n.capturedStart();
        token.EndsAt = n.capturedEnd();

        m_tokens.append(token);
    }

    return tokenInfoFromFactory(factory);
}

QString Tokenizer::Tokenize(const Tokens& tokens) const
{
    QString result;
    auto prevPos = 0;
    for(const auto& token : tokens) {
        result += QStringView(m_pattern).mid(prevPos, token.StartsAt - prevPos);
        if(token.ReplaceWith != nullptr) {
            result += token.ReplaceWith();
        }
        prevPos = token.EndsAt;
    }
    result += QStringView(m_pattern).mid(prevPos, m_pattern.size() - prevPos);
    return result;
}

Tokenizer::Tokens Tokenizer::tokenInfoFromFactory(const ProcessFactory& factory)
{
    Tokens result;
    for(const auto& token : m_tokens) {
        result.append(token);
        auto foundIt = factory.find(token.TokenName);
        if(foundIt != factory.end()) {
            result.last().ReplaceWith = foundIt.value();
        }
    }
    return result;
}
