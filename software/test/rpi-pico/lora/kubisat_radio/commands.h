#pragma once

#include <string>
#include <map>
#include "pico/stdlib.h"

enum class Command {
    GET_TIME,
    GET_VOLTAGE, 
    COMMANDS,
    UNKNOWN
};

// Command handler function declarations
void handleGetTime();
void handleGetVoltage();
void handleCommands();
void handleUnknownCommand();
void handleCommand(const std::string& message);
void sendMessage(std::string outgoing);

extern std::map<std::string, Command> commandMap;