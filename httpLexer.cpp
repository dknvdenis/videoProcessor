#include "httpLexer.h"
#include <tuple>

HttpLexer::HttpLexer(IStreamReaderPtr reader)
    : m_reader(reader)
{

}

std::vector<Token> HttpLexer::getTokens()
{
    std::vector<Token> result;

    while (true)
    {
        Token token = getToken();
        if (token.isUnknown())
            break;

        result.push_back(token);
    }

    return result;
}

Token HttpLexer::getToken()
{
    if (m_charIter.eof())
        m_charIter = m_reader->read();

    if (m_charIter.eof())
        return Token();

    TokenType type = TokenType::unknown;
    auto iter = m_charIter.begin;

    for (; iter != m_charIter.end; ++iter)
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
            if (m_charIter.begin != iter)
            {
                auto token = Token(TokenType::string, m_charIter.begin, iter);
                m_charIter.begin = iter;
                return token;
            }
            else
            {
                m_charIter.begin = iter + 1;
                return Token(type, iter, iter + 1);
            }
        }
    }

    if (type == TokenType::unknown)
    {
        auto token = Token(TokenType::string, m_charIter.begin, m_charIter.end);
        m_charIter.begin = iter;
        return token;
    }

    return Token();
}
