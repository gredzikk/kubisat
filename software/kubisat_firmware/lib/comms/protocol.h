// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <map>
#include <functional>
#include <vector>
#include <cstdint>
#include <iomanip>
#include "pin_config.h"
#include "PowerManager.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <cstring>
#include "utils.h"
#include "time.h"
#include "build_number.h"

/**
 * @defgroup Protocol Protocol
 * @brief Definitions for the communication protocol used by the satellite.
 */
/**
 * @def FRAME_BEGIN
 * @brief Defines the start marker for a communication frame.
 * @ingroup Protocol
 */
const std::string FRAME_BEGIN = "KBST";

/**
 * @def FRAME_END
 * @brief Defines the end marker for a communication frame.
 * @ingroup Protocol
 */
const std::string FRAME_END = "TSBK";

/**
 * @def DELIMITER
 * @brief Defines the delimiter used to separate fields within a communication frame.
 * @ingroup Protocol
 */
const char DELIMITER = ';';

/**
 * @enum ErrorCode
 * @brief Standard error codes for command responses
 * @ingroup Protocol
 */
enum class ErrorCode : uint8_t
{
    PARAM_UNNECESSARY,     // Parameter provided but not needed
    PARAM_REQUIRED,        // Required parameter missing
    PARAM_INVALID,         // Parameter has invalid format or value
    INVALID_OPERATION,     // Operation not allowed for this command
    NOT_ALLOWED,           // Operation not permitted
    INVALID_FORMAT,        // Input format is incorrect
    INVALID_VALUE,         // Value is outside expected range
    FAIL_TO_SET,           // Failed to set provided value
    INTERNAL_FAIL_TO_READ, // Failed to read from device in remote
    UNKNOWN_ERROR          // Generic error
};

/**
 * @enum OperationType
 * @brief Represents the type of operation being performed.
 * @ingroup Protocol
 */
enum class OperationType : uint8_t
{
    /** @brief Get data. */
    GET,
    /** @brief Set data. */
    SET,
    /** @brief Set command result. */
    RES,
    /** @brief Get command value. */
    VAL,
    /** @brief Sequence element response */
    SEQ,
    /** @brief Error occurred during command execution. */
    ERR,
    /** @brief Acknowledgement of a sequenced frame. */
    ACK,
};

/**
 * @enum ValueUnit
 * @brief Represents the unit of measurement for a payload value.
 * @ingroup Protocol
 */
enum class ValueUnit : uint8_t
{
    /** @brief Unit is undefined. */
    UNDEFINED,
    /** @brief Unit is seconds. */
    SECOND,
    /** @brief Unit is volts. */
    VOLT,
    /** @brief Unit is boolean. */
    BOOL,
    /** @brief Unit is date and time. */
    DATETIME,
    /** @brief Unit is text. */
    TEXT,
    /** @brief Unit is milliamperes. */
    MILIAMP,
    /** @brief Unit is degrees Celsius. */
    CELSIUS,
};

/**
 * @enum Interface
 * @brief Represents the communication interface being used.
 * @ingroup Protocol
 */
enum class Interface : uint8_t
{
    /** @brief UART interface. */
    UART,
    /** @brief LoRa interface. */
    LORA
};

/**
 * @struct Frame
 * @brief Represents a communication frame used for data exchange.
 *
 * @details This structure encapsulates the different components of a communication frame,
 *          including the header, direction, operation type, group ID, command ID, payload value, unit, and footer.
 *          It is used for both encoding and decoding messages.
 *
 * @note The `header` and `footer` fields are used to mark the beginning and end of the frame, respectively.
 * @note The `direction` field indicates the direction of the communication (0 = ground->sat, 1 = sat->ground).
 * @note The `operationType` field specifies the type of operation being performed (e.g., GET, SET, ANS, ERR, INF).
 * @note The `group` and `command` fields identify the specific command being executed.
 * @note The `value` field contains the payload data.
 * @note The `unit` field specifies the unit of measurement for the payload data.
 * @ingroup Protocol
 */
struct Frame
{
    std::string header; // Start marker
    uint8_t direction;  // 0 = ground->sat, 1 = sat->ground
    OperationType operationType;
    uint8_t group;      // Group ID
    uint8_t command;    // Command ID within group
    std::string value;  // Payload value
    std::string unit;   // Payload unit
    uint16_t seq = 0;   // Sequence number (0 if legacy / unset)
    uint16_t crc = 0;   // CRC16 of fields ADDR..SEQ
    bool has_integrity = false; // true when SEQ+CRC present and validated
    std::string footer; // End marker
};

/** Build CRC input string: dir;TYPE;group;cmd;value;unit;seq */
std::string frame_crc_payload(const Frame &frame);
uint16_t frame_compute_crc(const Frame &frame);

std::string error_code_to_string(ErrorCode code);
std::string operation_type_to_string(OperationType type);
OperationType string_to_operation_type(const std::string &str);
std::string value_unit_type_to_string(ValueUnit unit);

#endif

/** @} */