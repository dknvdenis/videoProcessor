#include "parserUtils.h"

ParserUtils::ParserUtils(ILexerPtr lexer)
    : m_lexer(lexer)
{

}

const Token &ParserUtils::token() const
{
    return m_currentToken;
}

bool ParserUtils::next()
{
    if (isStopSequenceReached())
        return false;

    if (!m_nextToken.isUnknown())
    {
        m_currentToken = m_nextToken;
        m_nextToken.clear();

        handleStopSequence();

        return true;
    }

    m_currentToken = m_lexer->getToken();
    if (m_currentToken.isUnknown())
        return false;

    if (m_mergeStringSequence)
    {
        // Merge a sequence of string tokens into one
        if (m_currentToken.type == TokenType::string)
        {
            while (true)
            {
                auto token = m_lexer->getToken();

                if (token.type != TokenType::string)
                {
                    m_nextToken = token;
                    break;
                }

                m_currentToken.value.end = token.value.end;
            }
        }
    }

    handleStopSequence();

    return true;
}

void ParserUtils::setMergeStringSequence(bool value)
{
    m_mergeStringSequence = value;
}

bool ParserUtils::expected(std::initializer_list<TokenType> tokens)
{
    for (TokenType type : tokens)
    {
        if (!next())
            return false;

        if (token().type != type)
            return false;
    }

    return true;
}

void ParserUtils::setStopSequence(const std::vector<TokenType> &tokens)
{
    m_stopSequence = tokens;
    clearStopSequenceReachedFlag();
}

bool ParserUtils::isStopSequenceReached() const
{
    return (!m_stopSequence.empty()
            && m_stopSequenceMatchScore == m_stopSequence.size());
}

void ParserUtils::clearStopSequenceReachedFlag()
{
    m_stopSequenceMatchScore = 0;
}

bool ParserUtils::matchStopSequence()
{
    if (m_currentToken.type == m_stopSequence[m_stopSequenceMatchScore])
    {
        m_stopSequenceMatchScore++;
        return true;
    }

    return false;
}

void ParserUtils::handleStopSequence()
{
    if (!m_stopSequence.empty())
    {
        if (!matchStopSequence())
        {
            m_stopSequenceMatchScore = 0;
            matchStopSequence();
        }
    }
}
