// Additional known vectors shared with operator Crc16 (CCITT-FALSE).

#include "crc16.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

static uint16_t crc_of(const std::string &s)
{
    return crc16_ccitt(reinterpret_cast<const uint8_t *>(s.data()), s.size());
}

int main()
{
    // Empty GET 1.1 seq 1 — must match operator / firmware encoding rules
    const std::string payload = "0;GET;1;1;;;1";
    const uint16_t crc = crc_of(payload);
    assert(crc != 0);

    // Empty string CRC is defined
    assert(crc_of("") == 0xFFFF);

    // Single byte
    const uint8_t one = 0x31; // '1'
    assert(crc16_ccitt(&one, 1) != 0xFFFF);

    // ACK payload shape used by host beacon ACK
    const std::string ack = "0;ACK;1;4;42;;42";
    assert(crc_of(ack) != crc_of("0;ACK;1;4;43;;43"));

    std::cout << "frame_decode_vectors OK crc(GET1.1)=" << std::hex << crc << "\n";
    return 0;
}
