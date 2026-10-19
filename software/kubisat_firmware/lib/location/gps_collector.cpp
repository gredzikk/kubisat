// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "lib/location/gps_collector.h"
#include "utils.h"
#include "pico/time.h"
#include "lib/location/NMEA/NMEA_data.h"
#include "event_manager.h"
#include <vector>
#include <ctime>
#include <cstring>
#include "DS3231.h"
#include <sstream>
#include "system_state_manager.h"
#include <cctype>
#include <array>

#define MAX_RAW_DATA_LENGTH 256

std::vector<std::string> splitString(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

static bool nmea_checksum_valid(const std::string &sentence)
{
    if (sentence.size() < 4 || sentence[0] != '$')
    {
        return false;
    }
    size_t star = sentence.find('*');
    if (star == std::string::npos || star + 2 >= sentence.size())
    {
        return false;
    }

    uint8_t checksum = 0;
    for (size_t i = 1; i < star; ++i)
    {
        checksum ^= static_cast<uint8_t>(sentence[i]);
    }

    auto hex_val = [](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        if (c >= 'A' && c <= 'F')
            return 10 + (c - 'A');
        return -1;
    };

    int hi = hex_val(sentence[star + 1]);
    int lo = hex_val(sentence[star + 2]);
    if (hi < 0 || lo < 0)
    {
        return false;
    }
    return checksum == static_cast<uint8_t>((hi << 4) | lo);
}

static std::string strip_checksum(const std::string &sentence)
{
    size_t star = sentence.find('*');
    if (star == std::string::npos)
    {
        return sentence;
    }
    return sentence.substr(0, star);
}

void collect_gps_data()
{
    if (SystemStateManager::get_instance().is_gps_collection_paused())
    {
        return;
    }

    std::array<char, MAX_RAW_DATA_LENGTH> raw_data_buffer;
    static int raw_data_index = 0;
    static bool had_fix = false;

    while (uart_is_readable(GPS_UART_PORT))
    {
        char c = uart_getc(GPS_UART_PORT);

        if (c == '\r' || c == '\n')
        {
            if (raw_data_index > 0)
            {
                raw_data_buffer[raw_data_index] = '\0';
                std::string message(raw_data_buffer.data());
                raw_data_index = 0;

                if (!nmea_checksum_valid(message))
                {
                    continue;
                }

                std::string body = strip_checksum(message);
                std::vector<std::string> tokens = splitString(body, ',');

                bool is_rmc = message.rfind("$GPRMC", 0) == 0 || message.rfind("$GNRMC", 0) == 0;
                bool is_gga = message.rfind("$GPGGA", 0) == 0 || message.rfind("$GNGGA", 0) == 0;

                if (is_rmc)
                {
                    NMEAData::get_instance().update_rmc_tokens(tokens);
                    bool fix_ok = tokens.size() > 2 && tokens[2] == "A";
                    if (fix_ok && !had_fix)
                    {
                        EventEmitter::emit(EventGroup::GPS, GPSEvent::LOCK);
                        had_fix = true;
                    }
                    else if (!fix_ok && had_fix)
                    {
                        EventEmitter::emit(EventGroup::GPS, GPSEvent::LOST);
                        had_fix = false;
                    }
                }
                else if (is_gga)
                {
                    NMEAData::get_instance().update_gga_tokens(tokens);
                    bool fix_ok = tokens.size() > 6 && tokens[6] != "0" && !tokens[6].empty();
                    if (fix_ok && !had_fix)
                    {
                        EventEmitter::emit(EventGroup::GPS, GPSEvent::LOCK);
                        had_fix = true;
                    }
                    else if (!fix_ok && had_fix)
                    {
                        EventEmitter::emit(EventGroup::GPS, GPSEvent::LOST);
                        had_fix = false;
                    }
                }
            }
        }
        else
        {
            if (raw_data_index < MAX_RAW_DATA_LENGTH - 1)
            {
                raw_data_buffer[raw_data_index++] = c;
            }
            else
            {
                raw_data_index = 0;
            }
        }
    }
}
