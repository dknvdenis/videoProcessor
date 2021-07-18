#include "httpLexer.h"
#include <cctype>

HttpLexer::HttpLexer()
{

}

std::pair<CharIter, std::vector<Token>>
HttpLexer::getTokens(CharIter begin, CharIter end)
{
    std::pair<CharIter, std::vector<Token>> result;

    auto &iter = result.first;
    auto &tokens = result.second;

    auto tokenBegin = begin;

    iter = begin;
    TokenType type = TokenType::unknown;

    auto findToken = [&] {
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
            if (tokenBegin != iter)
                tokens.push_back(Token(TokenType::string, tokenBegin, iter));

            tokens.push_back(Token(type, iter, iter + 1));

            tokenBegin = iter + 1;
        }
    };

    for (; iter != end; ++iter)
        findToken();

    if (type == TokenType::unknown)
        tokens.push_back(Token(TokenType::string, tokenBegin, end));

    return result;
}
