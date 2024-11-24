#include "DS3231.h"
#include <iomanip>
#include <sstream>

const std::array<std::string, 7> DS3231::WEEKDAYS = {
    "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

DS3231::DS3231(uint sda_pin, uint scl_pin, i2c_inst_t* i2c_port, 
               uint speed, uint8_t address) : i2c(i2c_port), address(address) {
    // Initialize I2C
    i2c_init(i2c, speed);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

bool DS3231::setTime(uint8_t sec, uint8_t min, uint8_t hour,
                    uint8_t weekday, uint8_t day, uint8_t month, uint16_t year) {
    uint8_t buffer[8];
    buffer[0] = 0;
    buffer[1] = bin2bcd(sec);
    buffer[2] = bin2bcd(min);
    buffer[3] = bin2bcd(hour);
    buffer[4] = bin2bcd(weekday);
    buffer[5] = bin2bcd(day);
    buffer[6] = bin2bcd(month);
    buffer[7] = bin2bcd(year - 2000);

    // Write time to RTC
    uint8_t reg = RTC_REGISTER;
    i2c_write_blocking(i2c, address, &reg, 1, true);
    return i2c_write_blocking(i2c, address, buffer, sizeof(buffer), false) == sizeof(buffer);
}

bool DS3231::getTime(uint8_t& sec, uint8_t& min, uint8_t& hour,
                    std::string& weekday, uint8_t& day, uint8_t& month, uint16_t& year) {
    uint8_t buffer[7];
    uint8_t reg = RTC_REGISTER;
    
    if (i2c_write_blocking(i2c, address, &reg, 1, true) != 1) {
        return false;
    }
    
    if (i2c_read_blocking(i2c, address, buffer, sizeof(buffer), false) != sizeof(buffer)) {
        return false;
    }

    sec = bcd2bin(buffer[0]);
    min = bcd2bin(buffer[1]);
    hour = bcd2bin(buffer[2]);
    weekday = WEEKDAYS[bcd2bin(buffer[3])];
    day = bcd2bin(buffer[4]);
    month = bcd2bin(buffer[5]);
    year = bcd2bin(buffer[6]) + 2000;
    
    return true;
}

std::string DS3231::getTimeString() {
    uint8_t sec, min, hour, day, month;
    uint16_t year;
    std::string weekday;
    
    if (!getTime(sec, min, hour, weekday, day, month, year)) {
        return "Error reading RTC";
    }
    
    std::stringstream ss;
    ss << preZero(hour) << ":" << preZero(min) << ":" << preZero(sec)
       << "      " << weekday << " " 
       << static_cast<int>(day) << "." 
       << static_cast<int>(month) << "." 
       << year;
    
    return ss.str();
}

uint8_t DS3231::bcd2bin(uint8_t val) {
    return ((val/16) * 10) + (val % 16);
}

uint8_t DS3231::bin2bcd(uint8_t val) {
    return ((val/10) * 16) + (val % 10);
}

std::string DS3231::preZero(uint8_t val) {
    if (val < 10) {
        return "0" + std::to_string(val);
    }
    return std::to_string(val);
}