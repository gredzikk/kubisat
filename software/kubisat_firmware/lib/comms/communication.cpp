// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
#include "LoRa-RP2040.h"
#include "pin_config.h"
#include "lora_defaults.h"
#include "system_state_manager.h"
#include "PowerManager.h"
#include "hardware/watchdog.h"
#include "pico/time.h"
#include <sstream>
#include <iomanip>

static constexpr uint32_t HEALTH_BEACON_INTERVAL_MS = 30000;
static uint32_t g_last_beacon_ms = 0;

bool initialize_radio()
{
    LoRa.set_pins(lora_cs_pin, lora_reset_pin, lora_irq_pin);
    const long frequency = KUBISAT_LORA_FREQUENCY_HZ;
    bool init_status = false;

    for (int attempt = 0; attempt < 3 && !init_status; ++attempt)
    {
        if (!LoRa.begin(frequency))
        {
            uart_print("LoRa init failed attempt " + std::to_string(attempt + 1), VerbosityLevel::WARNING);
            sleep_ms(50);
        }
        else
        {
            uart_print("LoRa initialized with frequency " + std::to_string(frequency) +
                           " sync=0x" + std::to_string(KUBISAT_LORA_SYNC_WORD),
                       VerbosityLevel::INFO);
            LoRa.onTxDone(lora_tx_done_callback);
            LoRa.enableCrc();
            LoRa.receive(0);
            init_status = true;
        }
    }

    EventEmitter::emit(EventGroup::COMMS, init_status ? CommsEvent::RADIO_INIT : CommsEvent::RADIO_ERROR);
    return init_status;
}

void lora_tx_done_callback()
{
    uart_print("LoRa transmission complete", VerbosityLevel::DEBUG);
    LoRa.receive(0);
}

void init_watchdog_and_boot_reason()
{
    if (watchdog_caused_reboot())
    {
        uart_print("Boot after watchdog reset", VerbosityLevel::WARNING);
        EventEmitter::emit(EventGroup::SYSTEM, SystemEvent::WATCHDOG_RESET);
    }

    // 8s timeout; fed from main loops
    watchdog_enable(8000, true);
}

void feed_watchdog()
{
    watchdog_update();
}

void maybe_send_health_beacon()
{
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - g_last_beacon_ms < HEALTH_BEACON_INTERVAL_MS)
    {
        return;
    }
    g_last_beacon_ms = now;

    auto &state = SystemStateManager::get_instance();
    float battery_v = PowerManager::get_instance().get_voltage_battery();
    uint32_t uptime_s = now / 1000;

    std::ostringstream payload;
    payload << std::fixed << std::setprecision(2)
            << "UP=" << uptime_s
            << ",RAD=" << (state.is_radio_init_ok() ? 1 : 0)
            << ",SD=" << (state.is_sd_card_mounted() ? 1 : 0)
            << ",ENV=" << (state.is_env_sensor_init_ok() ? 1 : 0)
            << ",LUX=" << (state.is_light_sensor_init_ok() ? 1 : 0)
            << ",IMU=" << (state.is_imu_sensor_init_ok() ? 1 : 0)
            << ",BAT=" << battery_v;

    Frame beacon = frame_build(OperationType::VAL, 1, 4, payload.str(), ValueUnit::TEXT);
    // Fire-and-forget until host reliably ACKs beacons (operator auto-ACK); avoids ~2s RX blackout every 30s.
    send_frame_lora(beacon, false);
    uart_print("Health beacon: " + payload.str(), VerbosityLevel::INFO);
}
