// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"

/**
 * @brief Initializes the LoRa radio module.
 * @return True if initialization was successful, false otherwise.
 * @details Sets the LoRa pins and attempts to begin LoRa communication at a specified frequency.
 *          Emits a CommsEvent::RADIO_INIT event on success or a CommsEvent::RADIO_ERROR event on failure.
 */
bool initialize_radio()
{
    LoRa.set_pins(lora_cs_pin, lora_reset_pin, lora_irq_pin);
    long frequency = 433E6;
    bool init_status = false;
    if (!LoRa.begin(frequency))
    {
        uart_print("LoRa init failed. Check your connections.", VerbosityLevel::WARNING);
        init_status = false;
    }
    else
    {
        uart_print("LoRa initialized with frequency " + std::to_string(frequency), VerbosityLevel::INFO);

        LoRa.onTxDone(lora_tx_done_callback);

        LoRa.receive(0);

        init_status = true;
    }

    EventEmitter::emit(EventGroup::COMMS, init_status ? CommsEvent::RADIO_INIT : CommsEvent::RADIO_ERROR);

    return init_status;
}

/**
 * @brief Callback function for LoRa transmission completion.
 * @details Prints a debug message to the UART and sets the LoRa module to receive mode.
 */
void lora_tx_done_callback()
{
    uart_print("LoRa transmission complete", VerbosityLevel::DEBUG);
    LoRa.receive(0);
}
