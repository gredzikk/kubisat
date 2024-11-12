#include "lib/ds3231.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include <string>

char buf[7];
std::string week[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#define I2C_PORT i2c0

int main()  
{
    stdio_init_all();
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    sleep_ms(20000);
    printf("DS3231 Test Program ...\n\n"); 

    DS3231 rtc(i2c0);
    datetime_t dt;

    // Set initial time
    dt.year = 2023;
    dt.month = 10;
    dt.day = 1;
    dt.dotw = 0; // Sunday
    dt.hour = 12;
    dt.min = 0;
    dt.sec = 0;
    rtc.setDatetime(&dt);

    while(1)  
    {  
        rtc.getDatetime(&dt);

        // Print date and time
        printf("20%02d/%02d/%02d  ", dt.year, dt.month, dt.day);
        printf("%02d:%02d:%02d  ", dt.hour, dt.min, dt.sec);
        printf("%s\n", week[dt.dotw]);

        sleep_ms(1000); 
    }  
    return 0;
}