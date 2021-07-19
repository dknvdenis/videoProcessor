#include "token.h"
#include  <cstring>

Token::Token(TokenType type_, CharIter begin_, CharIter end_)
    : type(type_), value(begin_, end_)
{}

bool Token::isUnknown() const
{
    return type == TokenType::unknown;
}

void Token::clear()
{
    type = TokenType::unknown; value = CharRange();
}

std::size_t Token::length() const
{
    return std::distance(value.begin, value.end);
}

bool Token::compare(const char *str) const
{
    int len = strlen(str);
    if (len == 0)
        return false;

    std::size_t i = 0;
    auto iter = value.begin;

    for (; i < len && iter != value.end; ++i, ++iter)
    {
        if (str[i] != *iter)
            return false;
    }

    return true;
}

std::string Token::toString() const
{
    return value.toString();
}
