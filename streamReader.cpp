#include "streamReader.h"

#include <cassert>
#include <poll.h>
#include <unistd.h>

#include "vpException.h"
#include "log.h"

//#define DEBUGE_INPUT

StreamReader::StreamReader(int socket, std::chrono::milliseconds timeout,
                           std::size_t bufferSize)
    : m_socket(socket),
      m_timeLeft(timeout),
      m_bufSize(bufferSize),
      m_buffer(new char[bufferSize])
{
    assert(socket > -1);
    assert(bufferSize > 0);
}

CharRange StreamReader::read()
{
    if (m_timeLeft.count() == 0)
        throw ReaderTimeout();

    if (m_bufPos == m_bufSize)
        throw ReaderBufferOverflow();

    pollfd pollInfo = {};
    pollInfo.fd = m_socket;
    pollInfo.events = POLLIN;

    using namespace std::chrono;
    auto startTime = steady_clock::now();

    int pollStatus = poll(&pollInfo, 1, m_timeLeft.count());

    auto now = steady_clock::now();
    auto dur = duration_cast<milliseconds>(now - startTime);

    if (dur > m_timeLeft)
        m_timeLeft = milliseconds(0);
    else
        m_timeLeft -= dur;

    if (pollStatus == 0)
        throw ReaderTimeout();
    else if (pollStatus == -1)
        throw ReaderError(errno);

    int count = ::read(m_socket, m_buffer.get() + m_bufPos, m_bufSize - m_bufPos);

    if (count == 0)
        return CharRange();
    else if (count == -1)
        throw ReaderError("Read error", errno);

    CharRange result(m_buffer.get() + m_bufPos, m_buffer.get() + m_bufPos + count);

#ifdef DEBUGE_INPUT
    PRINT_LOG("receive: " << result.toString());
#endif

    m_bufPos += count;

    return result;
}
