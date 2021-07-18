#pragma once

#include <string>

using CharIter = char*;

struct CharRange
{
    CharRange() = default;
    explicit CharRange(CharIter begin_, CharIter end_)
        : begin(begin_), end(end_)
    {}

    CharIter begin {nullptr};
    CharIter end {nullptr};

    std::string toString() const { return std::string(begin, end); }
};
