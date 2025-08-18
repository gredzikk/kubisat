// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "includes.h"

#define LOG_FILENAME "/log.txt"

void core1_entry()
{
    uart_print("Starting core 1", VerbosityLevel::DEBUG);
    EventEmitter::emit(EventGroup::SYSTEM, SystemEvent::CORE1_START);

    uint32_t last_telemetry_time = 0;
    uint32_t telemetry_collection_counter = 0;

    TelemetryManager::get_instance().init();

    while (true)
    {
        collect_gps_data();

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

    system("color");

    return true;
}

bool init_modules()
{
    bool radio_init_status = initialize_radio();
    SystemStateManager::get_instance().set_radio_init_ok(radio_init_status);

    bool sd_init_status = fs_init();
    SystemStateManager::get_instance().set_sd_card_mounted(sd_init_status);

    if (sd_init_status)
    {
        FILE *fp = fopen(LOG_FILENAME, "w");
        if (fp)
        {
            uart_print("Log file opened.", VerbosityLevel::DEBUG);
            int bytes_written = fprintf(fp, "System init started.\n");
            uart_print("Written " + std::to_string(bytes_written) + " bytes.", VerbosityLevel::DEBUG);
            int close_status = fclose(fp);
            uart_print("Close file status: " + std::to_string(close_status), VerbosityLevel::DEBUG);

            struct stat file_stat;
            if (stat(LOG_FILENAME, &file_stat) == 0)
            {
                size_t file_size = file_stat.st_size;
                uart_print("File size: " + std::to_string(file_size) + " bytes", VerbosityLevel::DEBUG);
            }
            else
            {
                uart_print("Failed to get file size", VerbosityLevel::ERROR);
            }

            uart_print("File path: /" + std::string(LOG_FILENAME), VerbosityLevel::DEBUG);
        }
        else
        {
            uart_print("Failed to open log file for writing.", VerbosityLevel::ERROR);
        }
    }

    if (sd_init_status)
    {
        uart_print("SD card init: OK", VerbosityLevel::DEBUG);
    }
    else
    {
        uart_print("SD card init: FAILED", VerbosityLevel::ERROR);
    }

    if (radio_init_status)
    {
        uart_print("Radio init: OK", VerbosityLevel::DEBUG);
    }
    else
    {
        uart_print("Radio init: FAILED", VerbosityLevel::ERROR);
    }

    Frame boot = frame_build(OperationType::RES, 0, 0, "HELLO");
    send_frame_lora(boot);

    uart_print("Initializing sensors...", VerbosityLevel::DEBUG);

    SensorWrapper &sensor_wrapper = SensorWrapper::get_instance();
    bool light_sensor_init = sensor_wrapper.sensor_init(SensorType::LIGHT, SENSORS_I2C_PORT);
    SystemStateManager::get_instance().set_light_sensor_init_ok(light_sensor_init);

    bool env_sensor_init = sensor_wrapper.sensor_init(SensorType::ENVIRONMENT, SENSORS_I2C_PORT);
    SystemStateManager::get_instance().set_env_sensor_init_ok(env_sensor_init);

    if (!light_sensor_init || !env_sensor_init)
    {
        uart_print("One or more sensors failed to initialize", VerbosityLevel::WARNING);
    }

    return sd_init_status && radio_init_status;
}

SystemOperatingMode define_system_operating_mode()
{
    // If system is running but measured battery voltage is below this threshold it means that power is sourced from USB
    static constexpr float BAT_VOLTAGE_THRESHOLD = 2.4f;
    // If system is running but measured current discharge is below this threshold it means that power is sourced from USB
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
    Frame boot = frame_build(OperationType::RES, 0, 0, "START_MODE_" + mode_string);
    send_frame_lora(boot);

    std::string boot_string = "System init completed @ " + std::to_string(to_ms_since_boot(get_absolute_time())) + " ms";
    uart_print(boot_string, VerbosityLevel::WARNING);

    gpio_put(PICO_DEFAULT_LED_PIN, true);

    while (true)
    {
        int packet_size = LoRa.parse_packet();
        if (packet_size)
        {
            on_receive(packet_size);
        }

        handle_uart_input();
    }

    return 0;
}