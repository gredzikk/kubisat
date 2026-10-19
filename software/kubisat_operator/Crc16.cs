using System.Text;

namespace kubisat_operator;

/// <summary>
/// CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF) — matches software/common/protocol/crc16.
/// </summary>
public static class Crc16 {
    public static ushort Compute(ReadOnlySpan<byte> data) {
        ushort crc = 0xFFFF;
        foreach (byte b in data) {
            crc ^= (ushort)(b << 8);
            for (int bit = 0; bit < 8; bit++) {
                if ((crc & 0x8000) != 0)
                    crc = (ushort)((crc << 1) ^ 0x1021);
                else
                    crc = (ushort)(crc << 1);
            }
        }
        return crc;
    }

    public static ushort ComputeAscii(string payload) {
        return Compute(Encoding.ASCII.GetBytes(payload));
    }

    public static string FormatHex(ushort crc) => crc.ToString("X4");
}
