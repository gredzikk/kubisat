// Host-side tests for CRC-16 frame integrity (mirrors firmware protocol rules).

#include "crc16.h"
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

static const char DELIM = ';';

static std::string crc_payload(int dir, const std::string &op, int group, int cmd,
                               const std::string &value, const std::string &unit, uint16_t seq)
{
    std::stringstream ss;
    ss << dir << DELIM << op << DELIM << group << DELIM << cmd << DELIM
       << value << DELIM << unit << DELIM << seq;
    return ss.str();
}

static uint16_t compute(const std::string &payload)
{
    return crc16_ccitt(reinterpret_cast<const uint8_t *>(payload.data()), payload.size());
}

static std::string encode(int dir, const std::string &op, int group, int cmd,
                          const std::string &value, const std::string &unit, uint16_t seq)
{
    const std::string payload = crc_payload(dir, op, group, cmd, value, unit, seq);
    const uint16_t crc = compute(payload);
    std::ostringstream ss;
    ss << "KBST" << DELIM << dir << DELIM << op << DELIM << group << DELIM << cmd << DELIM
       << value << DELIM << unit << DELIM << seq << DELIM
       << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << crc
       << DELIM << "TSBK";
    return ss.str();
}

static bool decode_and_check(const std::string &frame)
{
    // Minimal parser for tests
    if (frame.rfind("KBST;", 0) != 0 || frame.size() < 10)
    {
        return false;
    }
    if (frame.substr(frame.size() - 4) != "TSBK")
    {
        return false;
    }

    std::string body = frame.substr(5, frame.size() - 5 - 5); // strip KBST; and ;TSBK
    std::stringstream ss(body);
    std::string dir, op, group, cmd, value, unit, seq, crc;
    if (!std::getline(ss, dir, DELIM) || !std::getline(ss, op, DELIM) ||
        !std::getline(ss, group, DELIM) || !std::getline(ss, cmd, DELIM) ||
        !std::getline(ss, value, DELIM) || !std::getline(ss, unit, DELIM) ||
        !std::getline(ss, seq, DELIM) || !std::getline(ss, crc, DELIM))
    {
        return false;
    }

    std::string payload = dir + DELIM + op + DELIM + group + DELIM + cmd + DELIM + value + DELIM + unit + DELIM + seq;
    uint16_t expected = compute(payload);
    unsigned long got = std::stoul(crc, nullptr, 16);
    return expected == static_cast<uint16_t>(got);
}

int main()
{
    // Known-vector style: empty payload GET 1.1 seq 1
    const std::string f1 = encode(0, "GET", 1, 1, "", "", 1);
    assert(decode_and_check(f1));

    // Tamper CRC
    std::string bad = f1;
    size_t crc_pos = bad.rfind(';');
    crc_pos = bad.rfind(';', crc_pos - 1);
    // replace CRC digits
    size_t start = crc_pos + 1;
    bad.replace(start, 4, "0000");
    // If original CRC happened to be 0000, force different
    if (decode_and_check(bad))
    {
        bad.replace(start, 4, "FFFF");
    }
    assert(!decode_and_check(bad));

    // ACK-style payload
    const std::string ack = encode(1, "ACK", 1, 1, "1", "", 1);
    assert(decode_and_check(ack));

    // SEQ change changes CRC
    const std::string a = encode(0, "GET", 1, 3, "", "", 10);
    const std::string b = encode(0, "GET", 1, 3, "", "", 11);
    assert(a != b);
    assert(decode_and_check(a));
    assert(decode_and_check(b));

    std::cout << "frame_crc_tests OK\n";
    return 0;
}
