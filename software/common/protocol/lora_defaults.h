// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef KUBISAT_LORA_DEFAULTS_H
#define KUBISAT_LORA_DEFAULTS_H

/** Shared LoRa modem defaults for satellite + ground (keep in lockstep). */
constexpr long KUBISAT_LORA_FREQUENCY_HZ = 433E6;

/** Non-default sync word for network isolation (default SX127x is 0x12). */
constexpr int KUBISAT_LORA_SYNC_WORD = 0x4B;

#endif
