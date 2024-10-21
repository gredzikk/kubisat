#include <iostream>
#include "INA3221.h"

int main() {
    INA3221 sensor(INA3221::ADDR_40);

    if (!sensor.init(INA3221::ADDR_40)) {
        std::cerr << "Failed to initialize INA3221 sensor." << std::endl;
        return -1;
    }

    sensor.setConfiguration(INA3221::CHANNEL_1, INA3221::CHANNEL_2, INA3221::CHANNEL_3, INA3221::AVG_16, 
                            INA3221::VBUS_1100US, INA3221::SBUS_1100US, INA3221::SHUNT_AND_BUS_VOLTAGE_CONTINUOUS);

    while (true) {
        float busVoltage = sensor.getBusVoltage_V(INA3221::CHANNEL_1);
        float shuntVoltage = sensor.getShuntVoltage_mV(INA3221::CHANNEL_1);
        float current = sensor.getCurrent_mA(INA3221::CHANNEL_1);
        float power = sensor.getPower_W(INA3221::CHANNEL_1);

        std::cout << "Bus Voltage: " << busVoltage << " V" << std::endl;
        std::cout << "Shunt Voltage: " << shuntVoltage << " mV" << std::endl;
        std::cout << "Current: " << current << " mA" << std::endl;
        std::cout << "Power: " << power << " W" << std::endl;

        sleep(1);
    }

    return 0;
}