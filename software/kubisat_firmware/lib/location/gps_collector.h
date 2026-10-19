// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file gps_collector.h
 * @brief Header file for the GPS data collector module.
 *
 * @details This file defines the function `collect_gps_data`, which is
 *          responsible for reading raw NMEA sentences from the GPS UART,
 *          parsing them, and updating the NMEA data in the NMEAData singleton.
 *
 * @defgroup Location Location
 * @brief Classes for handling location data.
 *
 * @{
 */

#ifndef GPS_COLLECTOR_H
#define GPS_COLLECTOR_H

#include <string>
#include "hardware/uart.h"
#include "lib/location/NMEA/NMEA_data.h"
#include "pin_config.h"

/**
 * @brief Collects GPS data from the UART and updates the NMEA data.
 *
 * @details This function reads raw NMEA sentences from the GPS UART,
 *          parses them, and updates the RMC and GGA tokens in the
 *          NMEAData singleton. It also handles buffer overflow and
 *          checks for bootloader reset pending status.
 * @ingroup Location
 */
void collect_gps_data();

#endif
/** @} */