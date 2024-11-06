#include "stdlib.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "stdio.h"
#include "string.h"
#include "string"
//Inlude Library
#include "LoRa-RP2040.h"

using std::string;

/*   RP2040-zero
const int csPin = 1;          // LoRa radio chip select
const int resetPin = 29;       // LoRa radio reset
const int irqPin = 10;         // change for your board; must be a hardware interrupt pin
*/

//pi pico
const int csPin = 17;          // LoRa radio chip select
const int resetPin = 15;       // LoRa radio reset
const int irqPin = 14;         // change for your board; must be a hardware interrupt pin

string outgoing;              // outgoing message

uint8_t msgCount = 0;            // count of outgoing messages

uint8_t localAddress = 0xBB;     // address of this device
uint8_t destination = 0xFF;      // destination to send to

long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

void sendMessage(string outgoing) {
  int n = outgoing.length();
  char send[n+1];
  strcpy(send,outgoing.c_str());
  printf("Sending: %s (%d)\n",send, n);
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(n+1);                      // add payload length
  LoRa.print(send);                     // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  // read packet header uint8_ts:
  int recipient = LoRa.read();          // recipient address
  uint8_t sender = LoRa.read();            // sender address
  uint8_t incomingMsgId = LoRa.read();     // incoming msg ID
  uint8_t incomingLength = LoRa.read();    // incoming msg length

  string incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length() + 1) {   // check length for error
    printf("error: message length does not match length\n");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    printf("This message is not for me.\n");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  printf("Received from: 0x%x\n",sender);
  printf("Sent to: 0x%x\n", recipient);
  printf("Message ID: %d\n", incomingMsgId);
  printf("Message length: %d\n", incomingLength);
  printf("Message: %s\n", incoming.c_str());
  printf("RSSI: %d\n", LoRa.packetRssi());
  printf("Snr: %d\n", LoRa.packetSnr());
}

int main(){

  stdio_init_all();
  sleep_ms(20000);
  printf("\nLoRa Duplex\n");

  // override the default CS, reset, and IRQ pins (optional)
  // LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(500E6)) {             // initialize ratio at 915 MHz
    printf("LoRa init failed. Check your connections.\n");
    while (true);                       // if failed, do nothing
  }

  printf("LoRa init succeeded.\n");

  while (1) {
    if (to_ms_since_boot(get_absolute_time()) - lastSendTime > interval) {
      char message[256];
      snprintf(message, sizeof(message), "Hello, time: %ld ms", to_ms_since_boot(get_absolute_time()));
      sendMessage(message);
      lastSendTime = to_ms_since_boot(get_absolute_time());            // timestamp the message
      interval = (rand()%2000) + 1000;    // 2-3 seconds
    }
    // parse for a packet, and call onReceive with the result:
    onReceive(LoRa.parsePacket());
  }
  return 0;
}