#include "lib/ds3231.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <iostream>

int main() {
    // Initialize USB serial for output
    stdio_init_all();
    sleep_ms(5000); // Wait for serial to be ready
    
    printf("DS3231 RTC Test\n");

    // Create RTC object with default pins (SDA=16, SCL=17)
    DS3231 rtc;
    
    // Optionally set initial time (24-hour format)
    // Parameters: sec, min, hour, weekday(0-6), day, month, year
    rtc.setTime(0,41,20,4,14,11,2024); // 14:30:00 MON 15.03.2024
    
    while(1) {
        uint8_t sec, min, hour, day, month;
        uint16_t year;
        std::string weekday;
        
        if(rtc.getTime(sec, min, hour, weekday, day, month, year)) {
            printf("%02d:%02d:%02d %s %02d.%02d.%d\n", 
                   hour, min, sec, weekday.c_str(), day, month, year);
        }
        
        sleep_ms(1000); // Wait 1 second between readings
    }
    
    return 0;
}