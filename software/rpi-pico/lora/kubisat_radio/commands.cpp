#include "commands.h"
#include "LoRa/LoRa-RP2040.h"
#include <string>

extern uint8_t msgCount;
extern uint8_t localAddress;
extern uint8_t destination;

std::map<std::string, Command> commandMap = {
    {"get_time", Command::GET_TIME},
    {"get_voltage", Command::GET_VOLTAGE},
    {"commands", Command::COMMANDS}
};

void logMessage(const std::string& message);

void handleGetTime() {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    std::string response = "Current time: " + std::to_string(currentTime) + " ms";
    sendMessage(response);
}

void handleGetVoltage() {
    // Implement voltage reading here
    std::string response = "Voltage: 3.3V"; // Example response
    sendMessage(response);
}

void handleCommands() {
    std::string response = "Available commands:\n";
    for (const auto& [cmd, _] : commandMap) {
        response += "- " + cmd + "\n";
    }
    sendMessage(response);
}

void handleUnknownCommand() {
    std::string response = "Unknown command.\nFor available commands type 'commands'";
    sendMessage(response);
}

void handleCommand(const std::string& message) {
    auto it = commandMap.find(message);
    if (it != commandMap.end()) {
        switch (it->second) {
            case Command::GET_TIME:
                handleGetTime();
                break;
            case Command::GET_VOLTAGE:
                handleGetVoltage();
                break;
            case Command::COMMANDS:
                handleCommands();
                break;
            default:
                handleUnknownCommand();
                break;
        }
    } else {
        handleUnknownCommand();
    }
}