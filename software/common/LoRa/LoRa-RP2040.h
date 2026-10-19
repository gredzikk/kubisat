#ifndef LORA_H
#define LORA_H

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "string.h"

#ifndef LORA_DEFAULT_SPI
#define LORA_DEFAULT_SPI spi0
#endif
#ifndef LORA_DEFAULT_SPI_FREQUENCY
#define LORA_DEFAULT_SPI_FREQUENCY 8E6
#endif
#ifndef LORA_DEFAULT_SS_PIN
#define LORA_DEFAULT_SS_PIN 17
#endif
#ifndef LORA_DEFAULT_RESET_PIN
#define LORA_DEFAULT_RESET_PIN 22
#endif
#ifndef LORA_DEFAULT_DIO0_PIN
#define LORA_DEFAULT_DIO0_PIN 20
#endif
#ifndef PA_OUTPUT_RFO_PIN
#define PA_OUTPUT_RFO_PIN 0
#endif
#ifndef PA_OUTPUT_PA_BOOST_PIN
#define PA_OUTPUT_PA_BOOST_PIN 1
#endif

class LoRaClass {
public:
  LoRaClass();

  int begin(long frequency);
  void end();

  int beginPacket(int implicitHeader = false);
  int endPacket(bool async = false);

  int parse_packet(int size = 0);
  int parsePacket(int size = 0) { return parse_packet(size); }
  int packetRssi();
  float packetSnr();
  long packetFrequencyError();

  int rssi();

  virtual size_t write(uint8_t byte);
  virtual size_t write(const uint8_t *buffer, size_t size);
  size_t print(const char *str);

  virtual int available();
  virtual int read();
  virtual int peek();
  virtual void flush();

  void onCadDone(void (*callback)(bool));
  void onReceive(void (*callback)(int));
  void onTxDone(void (*callback)());

  void receive(int size = 0);
  void channelActivityDetection(void);

  void idle();
  void sleep();

  void setTxPower(int level, int outputPin = PA_OUTPUT_PA_BOOST_PIN);
  void setFrequency(long frequency);
  void setSpreadingFactor(int sf);
  void setSignalBandwidth(long sbw);
  void setCodingRate4(int denominator);
  void setPreambleLength(long length);
  void setSyncWord(int sw);
  void enableCrc();
  void disableCrc();
  void enableInvertIQ();
  void disableInvertIQ();

  void setOCP(uint8_t mA);

  void setGain(uint8_t gain);

  void crc() { enableCrc(); }
  void noCrc() { disableCrc(); }

  uint8_t random();

  void set_pins(int ss = LORA_DEFAULT_SS_PIN, int reset = LORA_DEFAULT_RESET_PIN, int dio0 = LORA_DEFAULT_DIO0_PIN);
  void setPins(int ss = LORA_DEFAULT_SS_PIN, int reset = LORA_DEFAULT_RESET_PIN, int dio0 = LORA_DEFAULT_DIO0_PIN)
  {
    set_pins(ss, reset, dio0);
  }
  void setSPI(spi_inst_t &spi);
  void setSPIFrequency(uint32_t frequency);

  void dumpRegisters();


private:
  void explicitHeaderMode();
  void implicitHeaderMode();

  void handleDio0Rise();
  bool isTransmitting();

  int getSpreadingFactor();
  long getSignalBandwidth();

  void setLdoFlag();

  uint8_t readRegister(uint8_t address);
  void writeRegister(uint8_t address, uint8_t value);
  uint8_t singleTransfer(uint8_t address, uint8_t value);

  static void onDio0Rise(uint, uint32_t);

private:
  // SPISettings _spiSettings;
  spi_inst_t *_spi;
  int _ss;
  int _reset;
  int _dio0;
  long _frequency;
  int _packetIndex;
  int _implicitHeaderMode;
  void (*_onReceive)(int);
  void (*_onCadDone)(bool);
  void (*_onTxDone)();
};

extern LoRaClass LoRa;

#endif // LORA_H
