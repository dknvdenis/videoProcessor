#include "httpLexer.h"
#include <tuple>
#include "log.h"

#if 0
    #define PRINT_TOKEN(token) \
        PRINT_LOG(">>> " << TokenTypeToString(token.type) \
                  << (token.type == TokenType::string \
                      ? "\t\"" + token.toString() + "\"" \
                      : ""))
#else
    #define PRINT_TOKEN(token)
#endif


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
    {
        PRINT_TOKEN(Token());
        return Token();
    }

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
                PRINT_TOKEN(token);
                return token;
            }
            else
            {
                m_charIter.begin = iter + 1;
                auto token = Token(type, iter, iter + 1);
                PRINT_TOKEN(token);
                return token;
            }
        }
    }

    if (type == TokenType::unknown)
    {
        auto token = Token(TokenType::string, m_charIter.begin, m_charIter.end);
        m_charIter.begin = iter;
        PRINT_TOKEN(token);
        return token;
    }

    PRINT_TOKEN(Token());
    return Token();
}
