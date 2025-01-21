#include "stdlib.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "stdio.h"
#include "string.h"
#include "string"
// Include Library
#include "LoRa/LoRa-RP2040.h"

using std::string;

#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 8
#define UART_RX_PIN 9

const int csPin = 1;	 // LoRa radio chip select
const int resetPin = 29; // LoRa radio reset
const int irqPin = 10;	 // change for your board; must be a hardware interrupt pin

string outgoing; // outgoing message

uint8_t msgCount = 0; // count of outgoing messages

uint8_t localAddress = 21; // address of this device
uint8_t destination = 37;  // destination to send to

long lastSendTime = 0;			   // last send time
long unsigned int interval = 2000; // interval between sends
long lastReceiveTime = 0;		   // last receive time

void logMessage(const string &message) {
    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[%lu ms] %s\n", timestamp, message.c_str());
    uart_puts(UART_ID, buffer);
}

void sendMessage(string outgoing)
{
	int n = outgoing.length();
	char send[n + 1];
	strcpy(send, outgoing.c_str());
	logMessage("ground to sat: " + string(send) + " [" + std::to_string(n) + "]");
	LoRa.beginPacket();		  // start packet
	LoRa.write(destination);  // add destination address
	LoRa.write(localAddress); // add sender address
	LoRa.write(msgCount);	  // add message ID
	LoRa.write(n + 1);		  // add payload length
	LoRa.print(send);		  // add payload
	LoRa.endPacket();		  // finish packet and send it
	msgCount++;				  // increment message ID
}

void onReceive(int packetSize)
{
	if (packetSize == 0)
		return; // if there's no packet, return
	// read packet header uint8_ts:
	int recipient = LoRa.read();		  // recipient address
	uint8_t sender = LoRa.read();		  // sender address
	uint8_t incomingMsgId = LoRa.read();  // incoming msg ID
	uint8_t incomingLength = LoRa.read(); // incoming msg length

	string incoming = "";

	while (LoRa.available())
	{
		incoming += (char)LoRa.read();
	}

	if (incomingLength != incoming.length() + 1)
	{ // check length for error
		printf("error: message length does not match length\n");
		return; // skip rest of function
	}

	// if the recipient isn't this device or broadcast,
	if (recipient != localAddress && recipient != 0xFF)
	{
		printf("This message is not for me.\n");
		return; // skip rest of function
	}

	logMessage("Received from: 0x" + std::to_string(sender));
	logMessage("Sent to: 0x" + std::to_string(recipient));
	logMessage("Message ID: " + std::to_string(incomingMsgId));
	logMessage("Message length: " + std::to_string(incomingLength));
	logMessage("Message: " + incoming);
	logMessage("RSSI: " + std::to_string(LoRa.packetRssi()));
	logMessage("Snr: " + std::to_string(LoRa.packetSnr()));
}

int main()
{
	stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    logMessage("\nPower on\n");

    int i = 5;
	while(i > 0) {
        logMessage("main loop in " + i);
        sleep_ms(1000);
        i--;
    }

    logMessage("\nground station\n");

    if (!LoRa.begin(433E6)) {
        logMessage("LoRa init failed. Check your connections.\n");
        while (true);
    }

    logMessage("init succeeded.\n");

    string userInput;
    bool waitingForReply = false;
    uint32_t messageStartTime = 0;
    const uint32_t REPLY_TIMEOUT = 5000;
    const string ACK = "ACK:";
    const string ERR = "ERR:";
    const string REPLY = "REPLY:";
    const string TIMEOUT = "TIMEOUT:";

    while (1) {
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());

        if (int packetSize = LoRa.parsePacket()) {
            if (waitingForReply) {
                logMessage(REPLY + "START");
                onReceive(packetSize);
                logMessage(REPLY + "END");
                waitingForReply = false;
            } else {
                onReceive(packetSize);
            }
        }

        if (waitingForReply && (currentTime - messageStartTime > REPLY_TIMEOUT)) {
            logMessage(TIMEOUT + userInput);
            waitingForReply = false;
        }

        if (!waitingForReply) {
            while (uart_is_readable(UART_ID)) {
                uint8_t ch = uart_getc(UART_ID);
                if (ch == '\n' || ch == '\r') {
                    if (!userInput.empty()) {
                        try {
                            sendMessage(userInput);
                            string ack = ACK + userInput + "\n";
                            uart_puts(UART_ID, ack.c_str());
                            waitingForReply = true;
                            messageStartTime = currentTime;
                        } catch (...) {
                            string err = ERR + userInput + "\n";
                            uart_puts(UART_ID, err.c_str());
                        }
                        userInput.clear();
                    }
                } else {
                    userInput += (char)ch;
                }
            }
        }

        sleep_ms(10);
    }
}