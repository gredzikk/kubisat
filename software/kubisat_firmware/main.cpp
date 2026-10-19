// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "includes.h"

#define LOG_FILENAME "/log.txt"

static constexpr uint32_t GPS_DUTY_ON_MS = 120000;
static constexpr uint32_t GPS_DUTY_OFF_MS = 180000;

void core1_entry()
{
    uart_print("Starting core 1", VerbosityLevel::DEBUG);
    EventEmitter::emit(EventGroup::SYSTEM, SystemEvent::CORE1_START);

    uint32_t last_telemetry_time = 0;
    uint32_t telemetry_collection_counter = 0;
    uint32_t gps_duty_epoch_ms = to_ms_since_boot(get_absolute_time());
    bool gps_duty_powered = true;

    TelemetryManager::get_instance().init();

    while (true)
    {
        feed_watchdog();
        collect_gps_data();

        // Battery-mode GPS duty cycle via power-enable pin
        if (SystemStateManager::get_instance().get_operating_mode() == SystemOperatingMode::BATTERY_POWERED)
        {
            uint32_t now = to_ms_since_boot(get_absolute_time());
            uint32_t elapsed = now - gps_duty_epoch_ms;
            if (gps_duty_powered && elapsed >= GPS_DUTY_ON_MS)
            {
                gpio_put(GPS_POWER_ENABLE_PIN, false);
                gps_duty_powered = false;
                gps_duty_epoch_ms = now;
                EventEmitter::emit(EventGroup::GPS, GPSEvent::POWER_OFF);
            }
            else if (!gps_duty_powered && elapsed >= GPS_DUTY_OFF_MS)
            {
                gpio_put(GPS_POWER_ENABLE_PIN, true);
                gps_duty_powered = true;
                gps_duty_epoch_ms = now;
                EventEmitter::emit(EventGroup::GPS, GPSEvent::POWER_ON);
            }
        }
        else if (!gps_duty_powered)
        {
            gpio_put(GPS_POWER_ENABLE_PIN, true);
            gps_duty_powered = true;
            EventEmitter::emit(EventGroup::GPS, GPSEvent::POWER_ON);
        }

        uint32_t currentTime = to_ms_since_boot(get_absolute_time());

        if (TelemetryManager::get_instance().is_telemetry_collection_time(currentTime, last_telemetry_time))
        {
            TelemetryManager::get_instance().collect_telemetry();
            telemetry_collection_counter++;

            if (TelemetryManager::get_instance().is_telemetry_flush_time(telemetry_collection_counter))
            {
                TelemetryManager::get_instance().flush_telemetry();
                telemetry_collection_counter = 0;
                uart_print("Telemetry flushed to SD", VerbosityLevel::INFO);
            }
        }

        if (SystemStateManager::get_instance().is_bootloader_reset_pending())
        {
            sleep_ms(100);
            uart_print("Entering BOOTSEL mode...", VerbosityLevel::WARNING);
            reset_usb_boot(0, 0);
        }

        sleep_ms(10);
    }
}

bool init_pico_hw()
{
    stdio_init_all();

    uart_init(DEBUG_UART_PORT, DEBUG_UART_BAUD_RATE);
    gpio_set_function(DEBUG_UART_TX_PIN, UART_FUNCSEL_NUM(DEBUG_UART_PORT, DEBUG_UART_TX_PIN));
    gpio_set_function(DEBUG_UART_RX_PIN, UART_FUNCSEL_NUM(DEBUG_UART_PORT, DEBUG_UART_RX_PIN));

    uart_init(GPS_UART_PORT, GPS_UART_BAUD_RATE);
    gpio_set_function(GPS_UART_TX_PIN, UART_FUNCSEL_NUM(GPS_UART_PORT, GPS_UART_TX_PIN));
    gpio_set_function(GPS_UART_RX_PIN, UART_FUNCSEL_NUM(GPS_UART_PORT, GPS_UART_RX_PIN));

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, true);

    i2c_init(MAIN_I2C_PORT, 400 * 1000);
    gpio_set_function(MAIN_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MAIN_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MAIN_I2C_SCL_PIN);
    gpio_pull_up(MAIN_I2C_SDA_PIN);

    gpio_init(GPS_POWER_ENABLE_PIN);
    gpio_set_dir(GPS_POWER_ENABLE_PIN, GPIO_OUT);
    gpio_put(GPS_POWER_ENABLE_PIN, true);

    i2c_init(SENSORS_I2C_PORT, 400 * 1000);
    gpio_set_function(SENSORS_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SENSORS_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SENSORS_I2C_SCL_PIN);
    gpio_pull_up(SENSORS_I2C_SDA_PIN);
    gpio_init(SENSORS_POWER_ENABLE_PIN);
    gpio_set_dir(SENSORS_POWER_ENABLE_PIN, GPIO_OUT);
    gpio_put(SENSORS_POWER_ENABLE_PIN, true);

    SystemStateManager::get_instance();
    EventEmitter::emit(EventGroup::GPS, GPSEvent::POWER_ON);

    return true;
}

static bool init_sensor_with_retry(SensorType type, int attempts)
{
    for (int i = 0; i < attempts; ++i)
    {
        if (SensorWrapper::get_instance().sensor_init(type, SENSORS_I2C_PORT))
        {
            return true;
        }
        sleep_ms(20);
    }
    return false;
}

bool init_modules()
{
    bool radio_init_status = initialize_radio();
    SystemStateManager::get_instance().set_radio_init_ok(radio_init_status);

    bool sd_init_status = fs_init();
    SystemStateManager::get_instance().set_sd_card_mounted(sd_init_status);

    if (sd_init_status)
    {
        FILE *fp = fopen(LOG_FILENAME, "a");
        if (fp)
        {
            fprintf(fp, "System init started.\n");
            fclose(fp);
        }
        else
        {
            uart_print("Failed to open log file for writing.", VerbosityLevel::ERROR);
        }
    }

    uart_print(sd_init_status ? "SD card init: OK" : "SD card init: FAILED",
               sd_init_status ? VerbosityLevel::DEBUG : VerbosityLevel::ERROR);
    uart_print(radio_init_status ? "Radio init: OK" : "Radio init: FAILED",
               radio_init_status ? VerbosityLevel::DEBUG : VerbosityLevel::ERROR);

    if (radio_init_status)
    {
        Frame boot = frame_build(OperationType::RES, 0, 0, "HELLO");
        send_frame_lora(boot, false);
    }

    uart_print("Initializing sensors...", VerbosityLevel::DEBUG);

    bool light_sensor_init = init_sensor_with_retry(SensorType::LIGHT, 3);
    SystemStateManager::get_instance().set_light_sensor_init_ok(light_sensor_init);

    bool env_sensor_init = init_sensor_with_retry(SensorType::ENVIRONMENT, 3);
    SystemStateManager::get_instance().set_env_sensor_init_ok(env_sensor_init);

    bool imu_sensor_init = init_sensor_with_retry(SensorType::IMU, 3);
    SystemStateManager::get_instance().set_imu_sensor_init_ok(imu_sensor_init);

    if (!light_sensor_init || !env_sensor_init || !imu_sensor_init)
    {
        uart_print("One or more sensors failed to initialize", VerbosityLevel::WARNING);
    }

    // Continue even if radio/SD failed — health beacon reports faults.
    return true;
}

SystemOperatingMode define_system_operating_mode()
{
    static constexpr float BAT_VOLTAGE_THRESHOLD = 2.4f;
    static constexpr uint8_t current_discharge_threshold = 40;
    float battery_voltage = PowerManager::get_instance().get_voltage_battery();
    uint8_t current_discharge = PowerManager::get_instance().get_current_draw();

    if (battery_voltage < BAT_VOLTAGE_THRESHOLD && current_discharge < current_discharge_threshold)
    {
        SystemStateManager::get_instance().set_operating_mode(SystemOperatingMode::USB_POWERED);
    }
    else
    {
        SystemStateManager::get_instance().set_operating_mode(SystemOperatingMode::BATTERY_POWERED);
    }

    return SystemStateManager::get_instance().get_operating_mode();
}

int main()
{
    init_pico_hw();
    init_watchdog_and_boot_reason();
    sleep_ms(100);
    init_modules();
    EventEmitter::emit(EventGroup::SYSTEM, SystemEvent::BOOT);
    sleep_ms(100);

    bool power_manager_init_status = PowerManager::get_instance().initialize();
    if (power_manager_init_status)
    {
        PowerManager::get_instance().configure(CONTINUOUS_MODE, INA3221_REG_CONF_AVG_16);
    }
    else
    {
        uart_print("Power manager init error", VerbosityLevel::ERROR);
    }

    SystemOperatingMode current_mode = define_system_operating_mode();

    multicore_launch_core1(core1_entry);

    gpio_put(PICO_DEFAULT_LED_PIN, false);

    std::string mode_string = (current_mode == SystemOperatingMode::USB_POWERED) ? "USB" : "BATTERY";
    uart_print("Operating mode: " + mode_string, VerbosityLevel::WARNING);
    if (SystemStateManager::get_instance().is_radio_init_ok())
    {
        Frame boot = frame_build(OperationType::RES, 0, 0, "START_MODE_" + mode_string);
        send_frame_lora(boot, false);
    }

    std::string boot_string = "System init completed @ " + std::to_string(to_ms_since_boot(get_absolute_time())) + " ms";
    uart_print(boot_string, VerbosityLevel::WARNING);

    gpio_put(PICO_DEFAULT_LED_PIN, true);

    while (true)
    {
        feed_watchdog();

        int packet_size = LoRa.parse_packet();
        if (packet_size)
        {
            on_receive(packet_size);
        }

        handle_uart_input();
        maybe_send_health_beacon();
    }

    return 0;
}
