#pragma once

#include <vector>
#include "httpLexer.h"

class ParserUtils
{
public:
    explicit ParserUtils(ILexerPtr lexer);

public:
    const Token &token() const;
    bool next();

    void setMergeStringSequence(bool value);

    bool expected(TokenType type);
    bool expected(std::initializer_list<TokenType> tokens);

    void setStopSequence(const std::vector<TokenType> &tokens);
    bool isStopSequenceReached() const;
    void clearStopSequenceReachedFlag();

    void setMaxLength(std::size_t bytes);
    void disableMaxLengthCheck();
    bool isMaxLengthReached() const;

private:
    ILexerPtr m_lexer;

    Token m_currentToken;

    bool m_mergeStringSequence {false};
    Token m_nextToken;

    std::vector<TokenType> m_stopSequence;
    int m_stopSequenceMatchScore {0};

    std::size_t m_maxLength {0};
    bool m_checkLength {false};

private:
    bool matchStopSequence();
    void handleStopSequence();
    void handleMaxLength();
};
