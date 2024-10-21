#include "Beastdevices_INA3221.h"
#include <iostream>
#include <unistd.h>
#include <iomanip> // For std::setw, std::setprecision, and std::fixed
#include <sstream> // For std::ostringstream
#include <deque>   // For std::deque

const float BATTERY_CAPACITY_MAH = 2000.0; // Battery capacity in mAh
const int AVERAGE_WINDOW_SIZE = 10;         // Number of measurements to average
const float BATTERY_MIN_VOLTAGE = 2.4;
const float BATTERY_MAX_VOLTAGE = 4.2;
const float TIME_INTERVAL_HOURS = 1.0 / 3600.0; // Time interval in hours (1 second)

int main() {
    // Create an instance of the INA3221 class with the I2C address 0x40
    INA3221 ina3221(INA3221_ADDR40_GND);

    // Initialize the INA3221 device
    ina3221.begin();

    // Set the device to continuous mode
    ina3221.setModeContinious();

    // Set power valid upper and lower limits
    ina3221.setPwrValidUpLimit(5500);
    ina3221.setPwrValidLowLimit(3300);

    ina3221.setBusMeasEnable();

    ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);

    std::deque<float> timeRemainingHistory;
float totalMahDrained = 0.0;
    while (true) {
        // Read the bus voltage and current for each channel
        float batteryVoltage = ina3221.getVoltage(INA3221_CH1);
        float usbCurrent = ina3221.getCurrent_mA(INA3221_CH1);
        float solarCurrent = ina3221.getCurrent_mA(INA3221_CH3);
        float railVoltage = ina3221.getVoltage(INA3221_CH2);
        float railCurrent = ina3221.getCurrent_mA(INA3221_CH2);

        // Calculate the sum of charging currents
        float chargingCurrent = usbCurrent + solarCurrent;

        // Calculate the total current flowing to or from the battery
        float totalCurrent = chargingCurrent - railCurrent;

        // Calculate the time remaining with the current draw
        std::string timeRemaining;
        float timeRemainingHours = 0.0;
        if (totalCurrent < 0) {
            timeRemainingHours = BATTERY_CAPACITY_MAH / -totalCurrent;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << timeRemainingHours;
            timeRemaining = oss.str() + " hours";
        } else {
            timeRemaining = "N/A";
        }

        // Update the time remaining history
        if (totalCurrent < 0) {
            if (timeRemainingHistory.size() >= AVERAGE_WINDOW_SIZE) {
                timeRemainingHistory.pop_front();
            }
            timeRemainingHistory.push_back(timeRemainingHours);
        }

        // Calculate the average time remaining
        float averageTimeRemaining = 0.0;
        if (!timeRemainingHistory.empty()) {
            float sum = 0.0;
            for (float t : timeRemainingHistory) {
                sum += t;
            }
            averageTimeRemaining = sum / timeRemainingHistory.size();
        }
        float mahDrainedLastSecond = totalCurrent * TIME_INTERVAL_HOURS;
            totalMahDrained += mahDrainedLastSecond;

        float batteryPercentage = ((batteryVoltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100;
            // Print the data in a table format
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Battery voltage: " << batteryVoltage << " V, "
                  << "USB current: " << std::setw(4) << static_cast<int>(usbCurrent) << " mA, "
                  << "Solar current: " << std::setw(4) << static_cast<int>(solarCurrent) << " mA, "
                  << "Sum of charging currents: " << std::setw(4) << static_cast<int>(chargingCurrent) << " mA\n";
        
        std::cout << "5V rail voltage: " << railVoltage << " V, "
                  << "Current draw: " << std::setw(4) << static_cast<int>(railCurrent) << " mA\n";

        std::cout << "Total current to/from battery: " << std::setw(4) << static_cast<int>(totalCurrent) << " mA\n";

        std::cout << "Time remaining with current draw: " << timeRemaining << "\n";
        std::cout << "Average time remaining (last 5 measurements): " << std::fixed << std::setprecision(1) << averageTimeRemaining << " hours\n";

        bool pvStatus = ina3221.getPwrValidAlertFlag();
        std::cout << "Power-Valid Status: " << (pvStatus ? "High" : "Low") << "\n";

        // Estimate offset voltage for debugging
        int32_t offsetVoltage = ina3221.estimateOffsetVoltage(INA3221_CH1, static_cast<uint32_t>(batteryVoltage * 1000));
        std::cout << "Estimated Offset Voltage: " << offsetVoltage << " uV / " << offsetVoltage / 1000 << " mV\n";
	std::cout << "Battery percentage: " << batteryPercentage << "\n" << std::flush;
    std::cout << "mAh drained in the last second: " << std::fixed << std::setprecision(6) << mahDrainedLastSecond << " mAh\n";
        std::cout << "Total mAh drained: " << std::fixed << std::setprecision(3) << totalMahDrained << " mAh\n";

        // Sleep for 100 milliseconds before reading again
        usleep(250000);

        // Clear the screen and move the cursor to the top
        std::cout << "\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F";
    }

    return 0;
}
