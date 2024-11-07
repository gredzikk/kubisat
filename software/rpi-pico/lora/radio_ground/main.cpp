#include "stdlib.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "stdio.h"
#include "string.h"
#include "string"
// Include Library
#include "LoRa/LoRa-RP2040.h"

using std::string;

const int csPin = 1;          // LoRa radio chip select
const int resetPin = 29;      // LoRa radio reset
const int irqPin = 10;        // change for your board; must be a hardware interrupt pin

string outgoing;              // outgoing message

uint8_t msgCount = 0;         // count of outgoing messages

uint8_t localAddress = 21;  // address of this device
uint8_t destination = 37;   // destination to send to

long lastSendTime = 0;        // last send time
long unsigned int interval = 2000; // interval between sends
long lastReceiveTime = 0;     // last receive time

void logMessage(const string& message) {
    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    printf("[%lu ms] %s\n", timestamp, message.c_str());
}

void sendMessage(string outgoing) {
  int n = outgoing.length();
  char send[n+1];
  strcpy(send, outgoing.c_str());
	logMessage("ground to sat: " + string(send) + " [" + std::to_string(n) +"]");
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(n + 1);                    // add payload length
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

	logMessage("Received from: 0x" + std::to_string(sender));
	logMessage("Sent to: 0x" + std::to_string(recipient));
	logMessage("Message ID: " + std::to_string(incomingMsgId));
	logMessage("Message length: " + std::to_string(incomingLength));
	logMessage("Message: " + incoming);
	logMessage("RSSI: " + std::to_string(LoRa.packetRssi()));
	logMessage("Snr: " + std::to_string(LoRa.packetSnr()));
}


int main() {
	stdio_init_all();
	sleep_ms(20000);
	logMessage("\nground station\n");

	if (!LoRa.begin(500E6)) {
		logMessage("LoRa init failed. Check your connections.\n");
		while (true);
	}

	logMessage("init succeeded.\n");
	string userInput;
	bool waitingForReply = false;
	uint32_t messageStartTime = 0;
	const uint32_t REPLY_TIMEOUT = 30000;
	bool promptShown = false;

	while (1) {
		uint32_t currentTime = to_ms_since_boot(get_absolute_time());

		if (int packetSize = LoRa.parsePacket()) {
			if (waitingForReply) {
				logMessage("\nReply received:");
				onReceive(packetSize);  // Pass the packet size we already got
				logMessage("End of reply");
				waitingForReply = false;
				promptShown = false;
			} else {
				onReceive(packetSize);  // Pass the packet size we already got
			}
		}

		if (waitingForReply && (currentTime - messageStartTime > REPLY_TIMEOUT)) {
			logMessage("No reply received within timeout period");
			waitingForReply = false;
			promptShown = false;
		}

		if (!waitingForReply) {
			if (!promptShown) {
				logMessage("\nEnter message: ");
				promptShown = true;
			}

			int c = getchar_timeout_us(0);
			if (c != PICO_ERROR_TIMEOUT) {
				if (c == '\r' || c == '\n') {
					if (!userInput.empty()) {
						printf("\n");
						sendMessage(userInput.c_str());
						waitingForReply = true;
						messageStartTime = currentTime;
						userInput.clear();
						logMessage("Waiting for reply...\n");
					}
				} else {
					userInput += (char)c;
					putchar(c);
				}
			}
		}

		sleep_ms(10);
	}
	return 0;
}
