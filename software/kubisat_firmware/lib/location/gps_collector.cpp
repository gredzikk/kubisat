// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file gps_collector.cpp
 * @brief Implementation of the GPS data collector module.
 *
 * @details This file implements the function `collect_gps_data`, which is
 *          responsible for reading raw NMEA sentences from the GPS UART,
 *          parsing them, and updating the NMEA data in the NMEAData singleton.
 *
 * @defgroup Location Location
 * @brief Classes for handling location data.
 *
 * @{
 */

#include "lib/location/gps_collector.h"
#include "utils.h"
#include "pico/time.h"
#include "lib/location/NMEA/nmea_data.h"
#include "event_manager.h"
#include <vector>
#include <ctime>
#include <cstring>
#include "DS3231.h"
#include <sstream>
#include "system_state_manager.h"

/**
 * @brief Maximum length of the raw data buffer for NMEA sentences.
 */
#define MAX_RAW_DATA_LENGTH 256

/**
 * @brief Splits a string into tokens based on a delimiter.
 *
 * @param[in] str The string to split.
 * @param[in] delimiter The delimiter character.
 * @return A vector of strings representing the tokens.
 * @ingroup Location
 */
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

/**
 * @brief Collects GPS data from the UART and updates the NMEA data.
 *
 * @details This function reads raw NMEA sentences from the GPS UART,
 *          parses them, and updates the RMC and GGA tokens in the
 *          NMEAData singleton. It also handles buffer overflow and
 *          checks for bootloader reset pending status.
 * @ingroup Location
 */
void collect_gps_data()
{

    if (SystemStateManager::get_instance().is_gps_collection_paused())
    {
        return;
    }

    std::array<char, MAX_RAW_DATA_LENGTH> raw_data_buffer;
    static int raw_data_index = 0;

    while (uart_is_readable(GPS_UART_PORT))
    {
        char c = uart_getc(GPS_UART_PORT);

        if (c == '\r' || c == '\n')
        {
            // End of message
            if (raw_data_index > 0)
            {
                raw_data_buffer[raw_data_index] = '\0';
                std::string message(raw_data_buffer.data());
                raw_data_index = 0;

                // Split the message into tokens
                std::vector<std::string> tokens = splitString(message, ',');

                // Update the global vectors based on the sentence type
                if (message.find("$GPRMC") == 0)
                {
                    NMEAData::get_instance().update_rmc_tokens(tokens);
                }
                else if (message.find("$GPGGA") == 0)
                {
                    NMEAData::get_instance().update_gga_tokens(tokens);
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
/** @} */