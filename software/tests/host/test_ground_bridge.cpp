// Host-side tests for ground bridge framing helpers.

#include "bridge_frame.h"
#include <cassert>
#include <iostream>
#include <string>

int main()
{
    using namespace kubisat_bridge;

    // Address filter
    assert(address_ok(21, 37, 21, 37));
    assert(!address_ok(37, 21, 21, 37));
    assert(!address_ok(21, 21, 21, 37));

    // Strip NULs
    std::string with_nul = std::string("KBST;1;VAL;1;1;ok;;;1;ABCD;TSBK") + '\0' + '\0';
    strip_trailing_nuls(with_nul);
    assert(with_nul.back() == 'K');
    assert(with_nul.find('\0') == std::string::npos);

    // Extract frame
    std::string payload = "noise KBST;1;VAL;1;1;hi;;;2;FFFF;TSBK trailing";
    std::string frame;
    assert(extract_frame(payload, false, 0, 0.0f, frame));
    assert(frame == "KBST;1;VAL;1;1;hi;;;2;FFFF;TSBK");

    // RSSI append
    frame.clear();
    assert(extract_frame("KBST;1;ACK;1;4;1;;;9;1234;TSBK", true, -87, 7.5f, frame));
    assert(frame.find("TSBK;-87;7.5") != std::string::npos ||
           frame.find("TSBK;-87;7") != std::string::npos);

    // Missing footer
    frame.clear();
    assert(!extract_frame("KBST;1;VAL;1;1;x", false, 0, 0.0f, frame));

    // Overflow-ish: empty
    assert(!extract_frame("", false, 0, 0.0f, frame));

    std::cout << "ground_bridge_tests OK\n";
    return 0;
}
