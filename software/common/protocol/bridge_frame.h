// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef KUBISAT_BRIDGE_FRAME_H
#define KUBISAT_BRIDGE_FRAME_H

#include "frame_markers.h"
#include <cstdint>
#include <cstring>
#include <string>

/** Host-testable helpers shared with ground bridge framing. */
namespace kubisat_bridge {

inline void strip_trailing_nuls(std::string &s)
{
    while (!s.empty() && s.back() == '\0')
    {
        s.pop_back();
    }
}

/**
 * Extract first KBST…TSBK frame. Optionally append ";rssi;snr" after the frame.
 * @return true if a frame was found
 */
inline bool extract_frame(const std::string &payload, bool append_rssi_snr,
                          int rssi, float snr, std::string &out_frame)
{
    size_t header_pos = payload.find(FRAME_BEGIN);
    if (header_pos == std::string::npos)
    {
        return false;
    }

    size_t footer_pos = payload.find(FRAME_END, header_pos);
    if (footer_pos == std::string::npos || footer_pos < header_pos)
    {
        return false;
    }

    out_frame = payload.substr(header_pos, footer_pos + std::strlen(FRAME_END) - header_pos);

    if (append_rssi_snr)
    {
        out_frame += FRAME_DELIMITER;
        out_frame += std::to_string(rssi);
        out_frame += FRAME_DELIMITER;
        out_frame += std::to_string(snr);
    }

    return true;
}

inline bool address_ok(uint8_t dest, uint8_t sender, uint8_t local, uint8_t remote)
{
    return dest == local && sender == remote;
}

} // namespace kubisat_bridge

#endif
