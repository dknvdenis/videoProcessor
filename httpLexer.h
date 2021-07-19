#pragma once

#include <vector>
#include <string>
#include <memory>

#include "streamReader.h"
#include "token.h"


class ILexer
{
public:
    virtual ~ILexer() = default;

public:
    virtual Token getToken() = 0;
};

using ILexerPtr = std::shared_ptr<ILexer>;

class HttpLexer : public ILexer
{
public:
    explicit HttpLexer(IStreamReaderPtr reader);

public:
    std::vector<Token> getTokens();
    Token getToken() override;

private:
    IStreamReaderPtr m_reader;
    CharRange m_charIter;
};
