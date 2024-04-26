#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "builders.h"

class Tokenizer
{
    using FReplaceWith = std::function<QString ()>;
    struct TokenInfo
    {
        Name TokenName;
        FReplaceWith ReplaceWith;
        qint32 StartsAt = -1;
        qint32 EndsAt = -1;
    };
public:
    using Tokens = QVector<TokenInfo>;
    using ProcessFactory = QHash<Name, FReplaceWith>;

    Tokenizer(const QString& pattern);

    static Factory<Name, FReplaceWith> CreateFactory() { return Factory<Name, FReplaceWith>(); }

    Tokens CreateTokens(const ProcessFactory& factory);
    QString Tokenize(const Tokens& tokens) const;

private:
    Tokens tokenInfoFromFactory(const ProcessFactory& factory);

private:
    QString m_pattern;
    QVector<TokenInfo> m_tokens;
};

#endif // TOKENIZER_H
