#pragma once

#include "charIter.h"

enum class TokenType
{
    unknown,
    string,
    space,
    equalSign,      // =
    ampersand,      // &
    lf,             // \n
    cr,             // \r
    doubleQuotes    // "
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

    case TokenType::doubleQuotes:
        return "doubleQuotes";
    }

    return "unknown";
}

struct Token
{
    Token() = default;
    explicit Token(TokenType type_, CharIter begin_, CharIter end_);

    TokenType type {TokenType::unknown};
    CharRange value;

    bool isUnknown() const;
    void clear();
    std::size_t length() const;
    bool compare(const char *str) const;
    std::string toString() const;
};
