// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef UTILS_PARSE_H
#define UTILS_PARSE_H

#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>

inline bool parse_int32(const std::string &text, int32_t &out)
{
    if (text.empty())
    {
        return false;
    }
    errno = 0;
    char *end = nullptr;
    long value = strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0' || errno == ERANGE || value < INT_MIN || value > INT_MAX)
    {
        return false;
    }
    out = static_cast<int32_t>(value);
    return true;
}

inline bool parse_uint16(const std::string &text, uint16_t &out)
{
    int32_t value = 0;
    if (!parse_int32(text, value) || value < 0 || value > 65535)
    {
        return false;
    }
    out = static_cast<uint16_t>(value);
    return true;
}

#endif
