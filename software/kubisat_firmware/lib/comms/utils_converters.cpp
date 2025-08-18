// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
/**
 * @file utils_converters.cpp
 * @brief Implements utility functions for converting between different data types.
 * @defgroup UtilsConverters Utility Converters
 */

/**
 * @brief Converts a ValueUnit to a string.
 * @param unit The ValueUnit to convert.
 * @return The string representation of the ValueUnit.
 * @ingroup UtilsConverters
 */
std::string value_unit_type_to_string(ValueUnit unit)
{
    switch (unit)
    {
    case ValueUnit::UNDEFINED:
        return "";
    case ValueUnit::SECOND:
        return "s";
    case ValueUnit::VOLT:
        return "V";
    case ValueUnit::BOOL:
        return "";
    case ValueUnit::DATETIME:
        return "";
    case ValueUnit::TEXT:
        return "";
    case ValueUnit::MILIAMP:
        return "mA";
    case ValueUnit::CELSIUS:
        return "C";
    default:
        return "";
    }
}

/**
 * @brief Converts an OperationType to a string.
 * @param type The OperationType to convert.
 * @return The string representation of the OperationType.
 * @ingroup UtilsConverters
 */
std::string operation_type_to_string(OperationType type)
{
    switch (type)
    {
    case OperationType::GET:
        return "GET";
    case OperationType::SET:
        return "SET";
    case OperationType::VAL:
        return "VAL";
    case OperationType::ERR:
        return "ERR";
    case OperationType::RES:
        return "RES";
    case OperationType::SEQ:
        return "SEQ";
    default:
        return "UNKNOWN";
    }
}

/**
 * @brief Converts a string to an OperationType.
 * @param str The string to convert.
 * @return The OperationType corresponding to the string. Defaults to GET if the string is not recognized.
 * @ingroup UtilsConverters
 */
OperationType string_to_operation_type(const std::string &str)
{
    if (str == "GET")
        return OperationType::GET;
    if (str == "SET")
        return OperationType::SET;
    if (str == "VAL")
        return OperationType::VAL;
    if (str == "ERR")
        return OperationType::ERR;
    if (str == "RES")
        return OperationType::RES;
    if (str == "SEQ")
        return OperationType::SEQ;
    return OperationType::GET; // Default to GET
}

/**
 * @brief Converts an ErrorCode to its string representation
 * @param code The error code
 * @return String representation of the error code
 * @ingroup UtilsConverters
 */
std::string error_code_to_string(ErrorCode code)
{
    switch (code)
    {
    case ErrorCode::PARAM_UNNECESSARY:
        return "PARAM_UNNECESSARY";
    case ErrorCode::PARAM_REQUIRED:
        return "PARAM_REQUIRED";
    case ErrorCode::PARAM_INVALID:
        return "PARAM_INVALID";
    case ErrorCode::INVALID_OPERATION:
        return "INVALID_OPERATION";
    case ErrorCode::NOT_ALLOWED:
        return "NOT_ALLOWED";
    case ErrorCode::INVALID_FORMAT:
        return "INVALID_FORMAT";
    case ErrorCode::INVALID_VALUE:
        return "INVALID_VALUE";
    case ErrorCode::FAIL_TO_SET:
        return "FAIL_TO_SET";
    case ErrorCode::INTERNAL_FAIL_TO_READ:
        return "INTERNAL_FAIL_TO_READ";
    default:
        return "UNKNOWN_ERROR";
    }
}

/** @} */