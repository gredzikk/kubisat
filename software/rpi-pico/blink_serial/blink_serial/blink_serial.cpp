#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define PIN_SDA 4
#define PIN_SCL 5

bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main() {
  stdio_init_all();

  // This example will use I2C0 on GPIO4 (SDA) and GPIO5 (SCL) running at 100kHz
  i2c_init(i2c0, 100 * 1000);
  gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
  gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(PIN_SDA);
  gpio_pull_up(PIN_SCL);

  while (true) {
    printf("\nI2C Bus Scan for INA3221\n");
    printf("Detected I2C devices:\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
      if (reserved_addr(addr)) {
        continue;
      }

      uint8_t rxdata;
      int ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);

      if (ret >= 0) {
        printf("Device found at address 0x%02x\n", addr);
      }
    }
    sleep_ms(1000);
  }
}