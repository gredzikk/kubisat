// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file NMEA_data.h
 * @brief Header file for the NMEAData class, which manages parsed NMEA sentences.
 *
 * @details This file defines the NMEAData class, a singleton that stores and
 *          provides access to parsed data from NMEA sentences received from a GPS module.
 *          It includes methods for updating and retrieving RMC and GGA tokens,
 *          as well as converting the data to a Unix timestamp.
 *
 * @defgroup Location Location
 * @brief Classes for handling location data.
 *
 * @{
 */

#ifndef NMEA_DATA_H
#define NMEA_DATA_H

#include <vector>
#include <string>
#include "pico/sync.h"
#include <cstring>

/**
 * @brief Manages parsed NMEA sentences.
 * @details This class is a singleton that stores and provides access to parsed
 *          data from NMEA sentences received from a GPS module. It includes
 *          methods for updating and retrieving RMC and GGA tokens, as well as
 *          converting the data to a Unix timestamp.
 * @ingroup Location
 */
class NMEAData
{
private:
    /** @brief Vector of tokens from the most recent RMC sentence. */
    std::vector<std::string> rmc_tokens_;
    /** @brief Vector of tokens from the most recent GGA sentence. */
    std::vector<std::string> gga_tokens_;
    /** @brief Mutex for thread-safe access to the RMC tokens. */
    mutex_t rmc_mutex_;
    /** @brief Mutex for thread-safe access to the GGA tokens. */
    mutex_t gga_mutex_;

    /**
     * @brief Private constructor for the singleton pattern.
     * @details Initializes the mutexes.
     */
    NMEAData()
    {
        mutex_init(&rmc_mutex_);
        mutex_init(&gga_mutex_);
    }

    /**
     * @brief Deleted copy constructor to prevent copying.
     */
    NMEAData(const NMEAData &) = delete;
    /**
     * @brief Deleted assignment operator to prevent assignment.
     */
    NMEAData &operator=(const NMEAData &) = delete;

public:
    /**
     * @brief Gets the singleton instance of the NMEAData class.
     * @return A reference to the singleton instance.
     */
    static NMEAData &get_instance()
    {
        static NMEAData instance;
        return instance;
    }

    /**
     * @brief Updates the RMC tokens with new data.
     * @param[in] tokens Vector of strings representing the RMC tokens.
     */
    void update_rmc_tokens(const std::vector<std::string> &tokens)
    {
        mutex_enter_blocking(&rmc_mutex_);
        rmc_tokens_ = tokens;
        mutex_exit(&rmc_mutex_);
    }

    /**
     * @brief Updates the GGA tokens with new data.
     * @param[in] tokens Vector of strings representing the GGA tokens.
     */
    void update_gga_tokens(const std::vector<std::string> &tokens)
    {
        mutex_enter_blocking(&gga_mutex_);
        gga_tokens_ = tokens;
        mutex_exit(&gga_mutex_);
    }

    /**
     * @brief Gets a copy of the RMC tokens.
     * @return A copy of the RMC tokens.
     */
    std::vector<std::string> get_rmc_tokens() const
    {
        mutex_enter_blocking(const_cast<mutex_t *>(&rmc_mutex_));
        std::vector<std::string> copy = rmc_tokens_;
        mutex_exit(const_cast<mutex_t *>(&rmc_mutex_));
        return copy;
    }

    /**
     * @brief Gets a copy of the GGA tokens.
     * @return A copy of the GGA tokens.
     */
    std::vector<std::string> get_gga_tokens() const
    {
        mutex_enter_blocking(const_cast<mutex_t *>(&gga_mutex_));
        std::vector<std::string> copy = gga_tokens_;
        mutex_exit(const_cast<mutex_t *>(&gga_mutex_));
        return copy;
    }
};

#endif
/** @} */