#pragma once

#include <stdexcept>

class ReaderError : public std::logic_error
{
public:
    explicit ReaderError(int code = 0)
        : logic_error("ReaderError"),
          m_errorCode(code)
    {}

    explicit ReaderError(const std::string &msg, int code = 0)
        : logic_error(msg),
          m_errorCode(code)
    {}

public:
    int getErrorCode() const { return m_errorCode; }

private:
    int m_errorCode;
};

class ReaderTimeout : public ReaderError
{
public:
    explicit ReaderTimeout()
        : ReaderError("ReaderTimeout")
    {}
};

class ReaderBufferOverflow : public ReaderError
{
public:
    explicit ReaderBufferOverflow()
        : ReaderError("ReaderBufferOverflow")
    {}
};
