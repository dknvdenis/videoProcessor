#pragma once

#include <memory>
#include <chrono>
#include "charIter.h"

class IStreamReader
{
public:
    virtual ~IStreamReader() = default;

public:
    virtual CharRange read() = 0;
};

class StreamReader : public IStreamReader
{
public:
    explicit StreamReader(int socket,
                          std::chrono::milliseconds timeout = std::chrono::seconds(5),
                          std::size_t bufferSize = 4096);

public:
    CharRange read() override;

private:
    int m_socket;
    std::chrono::milliseconds m_timeLeft;
    std::size_t m_bufSize;
    std::unique_ptr<char[]> m_buffer;
    std::size_t m_bufPos {0};
};

