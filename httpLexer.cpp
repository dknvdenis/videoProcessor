#include "httpLexer.h"
#include <tuple>

std::pair<CharIter, std::vector<Token>>
HttpLexer::getTokens(CharIter begin, CharIter end)
{
    std::pair<CharIter, std::vector<Token>> result;

    auto &iter = result.first;
    auto &tokens = result.second;

    iter = begin;

    while (iter != end)
    {
        Token token;
        std::tie(iter, token) = getToken(iter, end);
        tokens.push_back(token);
    }

    return result;
}

std::pair<CharIter, Token> HttpLexer::getToken(CharIter begin, CharIter end)
{
    TokenType type = TokenType::unknown;
    auto iter = begin;

    for (; iter != end; ++iter)
    {
        char sym = *iter;

        switch (sym)
        {
        case ' ':
            type = TokenType::space;
            break;

        case '=':
            type = TokenType::equalSign;
            break;

        case '&':
            type = TokenType::ampersand;
            break;

        case '\n':
            type = TokenType::lf;
            break;

        case '\r':
            type = TokenType::cr;
            break;

        default:
            type = TokenType::unknown;
        }

        if (type != TokenType::unknown)
        {
            if (begin != iter)
                return std::make_pair(iter, Token(TokenType::string, begin, iter));
            else
                return std::make_pair(iter + 1, Token(type, iter, iter + 1));
        }
    }

    if (type == TokenType::unknown)
        return std::make_pair(iter, Token(TokenType::string, begin, end));

    return std::make_pair(nullptr, Token());
}
