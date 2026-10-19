// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef KUBISAT_FRAME_MARKERS_H
#define KUBISAT_FRAME_MARKERS_H

/** Shared KubiSat frame markers (ground + satellite). */
constexpr const char *FRAME_BEGIN = "KBST";
constexpr const char *FRAME_END = "TSBK";
constexpr char FRAME_DELIMITER = ';';

/** SX127x max payload; leave 2 bytes for LoRa dest/sender addressing. */
constexpr int LORA_MAX_PACKET_SIZE = 255;
constexpr int LORA_ADDRESS_BYTES = 2;
constexpr int LORA_MAX_PAYLOAD_SIZE = LORA_MAX_PACKET_SIZE - LORA_ADDRESS_BYTES;

#endif
