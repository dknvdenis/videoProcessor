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

    if (isMaxLengthReached())
        return false;

    if (!m_nextToken.isUnknown())
    {
        m_currentToken = m_nextToken;
        m_nextToken.clear();

        handleStopSequence();
        handleMaxLength();

        return true;
    }

    m_currentToken = m_lexer->getToken();
    if (m_currentToken.isUnknown())
        return false;

    handleMaxLength();

    if (m_mergeStringSequence && !isMaxLengthReached())
    {
        // Merge a sequence of string tokens into one
        if (m_currentToken.type == TokenType::string)
        {
            while (true)
            {
                auto token = m_lexer->getToken();
                handleMaxLength();

                if (token.type != TokenType::string)
                {
                    m_nextToken = token;
                    break;
                }

                m_currentToken.value.end = token.value.end;
                if (isMaxLengthReached())
                    break;
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

bool ParserUtils::expected(TokenType type)
{
    if (!next())
        return false;

    if (token().type != type)
        return false;

    return true;
}

bool ParserUtils::expected(std::initializer_list<TokenType> tokens)
{
    for (TokenType type : tokens)
    {
        if (!expected(type))
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

void ParserUtils::setMaxLength(std::size_t bytes)
{
    m_maxLength = bytes;
    m_checkLength = true;
}

void ParserUtils::disableMaxLengthCheck()
{
    m_maxLength = 0;
    m_checkLength = false;
}

bool ParserUtils::isMaxLengthReached() const
{
    return m_checkLength && m_maxLength == 0;
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

void ParserUtils::handleMaxLength()
{
    if (!m_checkLength)
        return;

    std::size_t tokenLength = m_currentToken.length();
    if (tokenLength > m_maxLength)
        m_maxLength = 0;
    else
        m_maxLength -= tokenLength;
}
