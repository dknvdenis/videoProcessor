#pragma once

#include <vector>
#include <string>

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

using CharIter = char*;

struct Token
{
    Token() = default;
    explicit Token(TokenType type_, CharIter begin_, CharIter end_)
        : type(type_), begin(begin_), end(end_)
    {}

    TokenType type {TokenType::unknown};
    CharIter begin {nullptr};
    CharIter end {nullptr};
};

class HttpLexer
{
public:
    HttpLexer();

public:
    std::pair<CharIter, std::vector<Token>> getTokens(CharIter begin, CharIter end);
};
