// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file event_manager.cpp
 * @brief Implementation of the Event Manager and Event Emitter classes.
 *
 * @details This file implements the EventManager class, which provides a
 *          singleton instance for logging events to a circular buffer and
 *          saving them to persistent storage. The EventEmitter class provides
 *          a simple interface for emitting events throughout the system.
 *
 * @defgroup EventManagement Event Management
 * @brief Classes and enums for handling system events.
 *
 * @{
 */

#include "event_manager.h"
#include <cstdio>
#include "protocol.h"
#include "pico/multicore.h"
#include "communication.h"
#include "utils.h"
#include "DS3231.h"

/**
 * @brief Initializes the event manager.
 * @return True if initialization was successful, false otherwise.
 * @ingroup EventManagement
 */
bool EventManager::init()
{
    if (!SystemStateManager::get_instance().is_sd_card_mounted())
    {
        uart_print("Event manager initialized (storage not available)", VerbosityLevel::WARNING);
        return false;
    }

    FILE *file = fopen(EVENT_LOG_FILE, "w");
    if (!file)
    {
        file = fopen(EVENT_LOG_FILE, "w");
        if (file)
        {
            fclose(file);
            uart_print("Created new event log", VerbosityLevel::INFO);
        }
        else
        {
            uart_print("Failed to create event log", VerbosityLevel::ERROR);
            return false;
        }
    }
    else
    {
        fclose(file);
    }

    uart_print("Event manager initialized", VerbosityLevel::INFO);
    return true;
}

/**
 * @brief Logs an event to the event buffer.
 * @param[in] group Event group.
 * @param[in] event Event code.
 * @ingroup EventManagement
 */
void EventManager::log_event(uint8_t group, uint8_t event)
{
    mutex_enter_blocking(&eventMutex);

    uint32_t timestamp = DS3231::get_instance().get_local_time();
    uint16_t id = nextEventId++;

    EventLog &log = events[writeIndex];
    log.id = id;
    log.timestamp = timestamp;
    log.group = group;
    log.event = event;

    writeIndex = (writeIndex + 1) % EVENT_BUFFER_SIZE;
    if (eventCount < EVENT_BUFFER_SIZE)
    {
        eventCount++;
    }

    eventsSinceFlush++;

    mutex_exit(&eventMutex);

    std::string event_string = "Event: " + std::to_string(id) +
                               " Group: " + std::to_string(group) +
                               " Event: " + std::to_string(event);
    uart_print(event_string, VerbosityLevel::WARNING);

    if (eventsSinceFlush >= EVENT_FLUSH_THRESHOLD || group == static_cast<uint8_t>(EventGroup::POWER))
    {
        save_to_storage();
        eventsSinceFlush = 0;
    }
}

/**
 * @brief Gets an event from the event buffer.
 * @param[in] index Index of the event to retrieve.
 * @return A copy of the event log entry (safe after unlock).
 * @ingroup EventManagement
 */
EventLog EventManager::get_event(size_t index) const
{
    mutex_enter_blocking(const_cast<mutex_t *>(&eventMutex));
    if (index >= eventCount)
    {
        mutex_exit(const_cast<mutex_t *>(&eventMutex));
        return EventLog{};
    }

    size_t readIndex;
    if (eventCount == EVENT_BUFFER_SIZE)
    {
        readIndex = (writeIndex + index) % EVENT_BUFFER_SIZE;
    }
    else
    {
        readIndex = index;
    }
    EventLog event = events[readIndex];
    mutex_exit(const_cast<mutex_t *>(&eventMutex));
    return event;
}

/**
 * @brief Saves the event buffer to persistent storage.
 * @return True if the save was successful, false otherwise.
 * @ingroup EventManagement
 */
bool EventManager::save_to_storage()
{
    if (!SystemStateManager::get_instance().is_sd_card_mounted())
    {
        bool status = fs_init();
        if (!status)
        {
            return false;
        }
    }

    FILE *file = fopen(EVENT_LOG_FILE, "a");
    if (file)
    {
        size_t startIdx = (writeIndex >= eventsSinceFlush) ? writeIndex - eventsSinceFlush : EVENT_BUFFER_SIZE - (eventsSinceFlush - writeIndex);

        for (size_t i = 0; i < eventsSinceFlush; i++)
        {
            size_t idx = (startIdx + i) % EVENT_BUFFER_SIZE;
            fprintf(file, "%u;%lu;%u;%u\n",
                    events[idx].id,
                    events[idx].timestamp,
                    events[idx].group,
                    events[idx].event);
        }
        fclose(file);
        uart_print("Events saved to storage", VerbosityLevel::INFO);
        return true;
    }
    return false;
}

/** @} */