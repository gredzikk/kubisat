// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file event_manager.h
 * @brief Header file for the Event Manager and Event Emitter classes.
 *
 * @details This file defines the classes and enumerations necessary for
 *          managing and emitting system events. The EventManager class
 *          provides a singleton instance for logging events to a circular
 *          buffer and saving them to persistent storage. The EventEmitter
 *          class provides a simple interface for emitting events throughout
 *          the system.
 *
 * @defgroup EventManagement Event Management
 * @brief Classes and enums for handling system events.
 *
 * @{
 */

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "PowerManager.h"
#include <cstdint>
#include <string>
#include "pico/mutex.h"
#include "storage.h"
#include "utils.h"
#include "system_state_manager.h"

/**
 * @brief Size of the event buffer.
 */
#define EVENT_BUFFER_SIZE 100

/**
 * @brief Number of events to accumulate before flushing to storage.
 */
#define EVENT_FLUSH_THRESHOLD 10

/**
 * @brief Path to the event log file.
 */
#define EVENT_LOG_FILE "/event_log.csv"

/**
 * @brief Enumeration of event groups.
 * @details Defines the different categories of events that can be logged.
 * @ingroup EventManagement
 */
enum class EventGroup : uint8_t
{
    /** @brief System-level events. */
    SYSTEM = 0x00,
    /** @brief Power management events. */
    POWER = 0x01,
    /** @brief Communications events. */
    COMMS = 0x02,
    /** @brief GPS events. */
    GPS = 0x03,
    /** @brief Clock events. */
    CLOCK = 0x04
};

/**
 * @brief Enumeration of system events.
 * @details Defines specific system-level events.
 * @ingroup EventManagement
 */
enum class SystemEvent : uint8_t
{
    /** @brief System boot event. */
    BOOT = 0x01,
    /** @brief System shutdown event. */
    SHUTDOWN = 0x02,
    /** @brief Watchdog reset event. */
    WATCHDOG_RESET = 0x03,
    /** @brief Core 1 start event. */
    CORE1_START = 0x04,
    /** @brief Core 1 stop event. */
    CORE1_STOP = 0x05
};

/**
 * @brief Enumeration of power events.
 * @details Defines specific power management events.
 * @ingroup EventManagement
 */
enum class PowerEvent : uint8_t
{
    /** @brief Low battery event. */
    BATTERY_LOW = 0x01,
    /** @brief Overcharge event. */
    BATTERY_FULL = 0x02,
    /** @brief Power falling event. */
    POWER_FALLING = 0x03,
    /** @brief Power normal event. */
    BATTERY_NORMAL = 0x04,
    /** @brief Solar charging active event. */
    SOLAR_ACTIVE = 0x05,
    /** @brief Solar charging inactive event. */
    SOLAR_INACTIVE = 0x06,
    /** @brief USB connected event. */
    USB_CONNECTED = 0x07,
    /** @brief USB disconnected event. */
    USB_DISCONNECTED = 0x08,
    /** @brief Current balance negative */
    DISCHARGING = 0x09,
    /** @brief Current balance positive */
    CHARGING = 0x0A,
};

/**
 * @brief Enumeration of communications events.
 * @details Defines specific communications events.
 * @ingroup EventManagement
 */
enum class CommsEvent : uint8_t
{
    /** @brief Radio initialization event. */
    RADIO_INIT = 0x01,
    /** @brief Radio error event. */
    RADIO_ERROR = 0x02,
    /** @brief Message received event. */
    MSG_RECEIVED = 0x03,
    /** @brief Message sent event. */
    MSG_SENT = 0x04,
    /** @brief UART error event. */
    UART_ERROR = 0x06
};

/**
 * @brief Enumeration of GPS events.
 * @details Defines specific GPS events.
 * @ingroup EventManagement
 */
enum class GPSEvent : uint8_t
{
    /** @brief GPS lock event. */
    LOCK = 0x01,
    /** @brief GPS lost event. */
    LOST = 0x02,
    /** @brief GPS error event. */
    ERROR = 0x03,
    /** @brief GPS power on event. */
    POWER_ON = 0x04,
    /** @brief GPS power off event. */
    POWER_OFF = 0x05,
    /** @brief GPS data ready event. */
    DATA_READY = 0x06,
    /** @brief GPS pass-through start event. */
    PASS_THROUGH_START = 0x07,
    /** @brief GPS pass-through end event. */
    PASS_THROUGH_END = 0x08
};

/**
 * @brief Enumeration of clock events.
 * @details Defines specific clock-related events.
 * @ingroup EventManagement
 */
enum class ClockEvent : uint8_t
{
    /** @brief Clock changed event. */
    CHANGED = 0x01,
    /** @brief GPS sync event. */
    GPS_SYNC = 0x02,
    /** @brief GPS sync data not ready event. */
    GPS_SYNC_DATA_NOT_READY = 0x03
};

/**
 * @brief Structure for storing event log data.
 * @details Represents a single event log entry with an ID, timestamp, group, and event code.
 * @ingroup EventManagement
 */
class EventLog
{
public:
    /** @brief Timestamp of the event in milliseconds since boot. */
    uint32_t timestamp;
    /** @brief Unique identifier for the event. */
    uint16_t id;
    /** @brief Event group. */
    uint8_t group;
    /** @brief Event code. */
    uint8_t event;
} __attribute__((packed));

/**
 * @brief Manages event logging and storage.
 * @details This class provides a singleton instance for logging events to a
 *          circular buffer and saving them to persistent storage. It ensures
 *          thread-safe access to the event log and provides methods for
 *          initializing, logging, retrieving, saving, and loading events.
 * @ingroup EventManagement
 */
class EventManager
{
private:
    EventLog events[EVENT_BUFFER_SIZE];
    size_t eventCount;
    size_t writeIndex;
    mutex_t eventMutex;
    uint16_t nextEventId;
    size_t eventsSinceFlush;

    EventManager() : eventCount(0),
                     writeIndex(0),
                     nextEventId(0),
                     eventsSinceFlush(0)
    {
        mutex_init(&eventMutex);
    }

    EventManager(const EventManager &) = delete;
    EventManager &operator=(const EventManager &) = delete;

public:
    /**
     * @brief Gets the singleton instance of the EventManager class.
     * @return A reference to the singleton instance.
     */
    static EventManager &get_instance()
    {
        static EventManager instance;
        return instance;
    }

    /**
     * @brief Initializes the event manager.
     * @return True if initialization was successful, false otherwise.
     */
    bool init();

    /**
     * @brief Logs an event to the event buffer.
     * @param[in] group Event group.
     * @param[in] event Event code.
     */
    void log_event(uint8_t group, uint8_t event);

    /**
     * @brief Gets an event from the event buffer (by value; safe across cores).
     * @param[in] index Index of the event to retrieve.
     * @return A copy of the event log entry, or a zeroed entry if out of range.
     */
    EventLog get_event(size_t index) const;

    /**
     * @brief Gets the number of events in the buffer.
     * @return The number of events in the buffer.
     */
    size_t get_event_count() const { return eventCount; }

    /**
     * @brief Saves the event buffer to persistent storage.
     * @return True if the save was successful, false otherwise.
     */
    bool save_to_storage();
};

/**
 * @brief Provides a simple interface for emitting events.
 * @details This class provides a static method for emitting events, which
 *          logs the event to the EventManager.
 * @ingroup EventManagement
 */
class EventEmitter
{
public:
    /**
     * @brief Emits an event.
     * @param[in] group Event group.
     * @param[in] event Event code.
     * @tparam T Type of the event enumeration.
     */
    template <typename T>
    static void emit(EventGroup group, T event)
    {
        EventManager::get_instance().log_event(
            static_cast<uint8_t>(group),
            static_cast<uint8_t>(event));
    }
};

#endif // EVENT_MANAGER_H
       /** @} */