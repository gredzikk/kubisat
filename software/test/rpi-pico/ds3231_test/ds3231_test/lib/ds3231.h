#ifndef DS3231_H
#define DS3231_H

#include <string>
#include <array>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

class DS3231 {
public:
    // Weekday names
    static const std::array<std::string, 7> WEEKDAYS;
    
    // Constructor
    DS3231(uint sda_pin = 4, uint scl_pin = 5, i2c_inst_t* i2c_port = i2c0, 
           uint speed = 400*1000, uint8_t address = 0x68);
    
    // Set time using binary-coded decimal format
    bool setTime(uint8_t sec, uint8_t min, uint8_t hour, 
                 uint8_t weekday, uint8_t day, uint8_t month, uint16_t year);
    
    // Get time in two formats
    bool getTime(uint8_t& sec, uint8_t& min, uint8_t& hour,
                std::string& weekday, uint8_t& day, uint8_t& month, uint16_t& year);
    std::string getTimeString();

private:
    i2c_inst_t* i2c;
    uint8_t address;
    static constexpr uint8_t RTC_REGISTER = 0x00;
    
    // Utility functions
    uint8_t bcd2bin(uint8_t val);
    uint8_t bin2bcd(uint8_t val);
    std::string preZero(uint8_t val);
};

#endif