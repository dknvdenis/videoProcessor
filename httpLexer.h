#pragma once

#include <vector>
#include <string>

#include "charIter.h"
#include "streamReader.h"

enum class TokenType
{
    unknown,
    string,
    space,
    equalSign,      // =
    ampersand,      // &
    lf,             // \n
    cr              // \r
};

inline std::string TokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::string:
        return "string";

    case TokenType::space:
        return "space";

    case TokenType::equalSign:
        return "equalSign";

    case TokenType::ampersand:
        return "ampersand";

    case TokenType::lf:
        return "LF";

    case TokenType::cr:
        return "CR";
    }

    return "unknown";
}

struct Token
{
    Token() = default;
    explicit Token(TokenType type_, CharIter begin_, CharIter end_)
        : type(type_), value(begin_, end_)
    {}

    TokenType type {TokenType::unknown};
    CharRange value;

    bool isUnknown() const { return type == TokenType::unknown; }
};

class HttpLexer
{
public:
    explicit HttpLexer(IStreamReaderPtr reader);

public:
    std::vector<Token> getTokens();
    Token getToken();

private:
    IStreamReaderPtr m_reader;
    CharRange m_charIter;
};
