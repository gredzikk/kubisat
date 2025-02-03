#include "ds3231/ds3231.h"
#include <stdio.h>
#include "pico/stdlib.h"

/*
     the first verison use i2c1(GP6,GP7),
    and the new vesion use i2c0(GP20,GP21).
    change it in ds3231/ds3231.h file
*/

char buf[7];
char *week[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int main()  
{
    stdio_init_all();
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(20, GPIO_FUNC_I2C);
    gpio_set_function(21, GPIO_FUNC_I2C);
    gpio_pull_up(20);
    gpio_pull_up(21);
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