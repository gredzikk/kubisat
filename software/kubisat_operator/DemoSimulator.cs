using System.Globalization;

namespace kubisat_operator;

/// <summary>
/// Generates plausible satellite responses for the virtual "demo" port.
/// Accepts full SEQ+CRC frames and legacy frames.
/// </summary>
public static class DemoSimulator {
    public const string PortName = "demo";

    private static int _verbosity = 3;
    private static int _gpsPower = 1;
    private static int _timezoneOffset;
    private static readonly Random _random = new();

    public static string? BuildResponse(string outgoingFrame) {
        string trimmed = outgoingFrame.Trim();
        if (trimmed == "?")
            return null; // RSSI arm only — no response

        string[] parts = trimmed.Split(';');
        if (parts.Length < 6 || parts[0] != "KBST")
            return null;

        string operation = parts[2];
        if (operation == "ACK")
            return null;

        if (!byte.TryParse(parts[3], out byte groupId) || !byte.TryParse(parts[4], out byte commandId))
            return null;

        string parameter = parts[5];
        ushort seq = 1;
        bool hasIntegrity = parts.Length >= 10 && parts[^1] == "TSBK" && ushort.TryParse(parts[7], out seq);

        string responseType;
        string value;

        if (operation == "SET") {
            ApplySet(groupId, commandId, parameter);
            responseType = "RES";
            value = "OK";
        }
        else {
            responseType = "VAL";
            value = BuildGetValue(groupId, commandId, parameter);
        }

        string response = EncodeResponse(responseType, groupId, commandId, value, seq, hasIntegrity);

        // Optional RSSI append is driven by operator sending "?" separately;
        // demo attaches debug when RadioDebug was used historically — skip here.
        return response;
    }

    private static string EncodeResponse(string responseType, byte groupId, byte commandId, string value, ushort seq, bool withIntegrity) {
        if (!withIntegrity)
            return $"KBST;1;{responseType};{groupId};{commandId};{value};TSBK";

        string unit = "";
        string crcPayload = $"1;{responseType};{groupId};{commandId};{value};{unit};{seq}";
        string crcHex = Crc16.FormatHex(Crc16.ComputeAscii(crcPayload));
        return $"KBST;{crcPayload};{crcHex};TSBK";
    }

    private static void ApplySet(byte groupId, byte commandId, string parameter) {
        switch (groupId) {
            case 1 when commandId == 8:
                if (int.TryParse(parameter, out int verbosity))
                    _verbosity = verbosity;
                break;
            case 3 when commandId == 1:
                if (int.TryParse(parameter, out int offset))
                    _timezoneOffset = offset;
                break;
            case 7 when commandId == 1:
                if (int.TryParse(parameter, out int power))
                    _gpsPower = power;
                break;
        }
    }

    private static string BuildGetValue(byte groupId, byte commandId, string parameter) {
        return (groupId, commandId) switch {
            (1, 0) => "1.0-1.1-1.2-1.3-1.4-1.5-1.8-1.9-2.0-2.1-2.2-3.0-3.1-3.4-5.1-5.2-7.1-7.2-8.2-8.3-8.4",
            (1, 1) => "1.4.2-demo",
            (1, 2) => "USB",
            (1, 3) => ((long)(DateTime.UtcNow - DateTime.UtcNow.Date).TotalSeconds + 86400).ToString(),
            (1, 4) => "UP=3600,RAD=1,SD=1,ENV=1,LUX=1,IMU=1,BAT=3.95",
            (1, 8) => _verbosity.ToString(),
            (2, 0) => "1",
            (2, 1) => "0.02,-0.01,0.98",
            (2, 2) => "0.1,-0.2,0.05",
            (3, 0) => DateTimeOffset.UtcNow.ToUnixTimeSeconds().ToString(),
            (3, 1) => _timezoneOffset.ToString(),
            (3, 4) => (22.5 + _random.NextDouble()).ToString("F1", CultureInfo.InvariantCulture),
            (5, 1) => BuildEventLog(parameter),
            (5, 2) => "12",
            (7, 1) => _gpsPower.ToString(),
            (8, 2) => BuildTelemetryCsv(),
            (8, 3) => BuildSensorCsv(),
            (8, 4) => BuildTelemetryCsv() + "\n" + BuildTelemetryCsv(),
            _ => "0"
        };
    }

    private static string BuildEventLog(string parameter) {
        int count = 5;
        if (int.TryParse(parameter, out int requested) && requested > 0)
            count = Math.Min(requested == 0 ? 12 : requested, 12);
        if (parameter == "0")
            count = 12;

        long now = DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        var events = new List<string>();
        for (int i = 0; i < count; i++) {
            int eventId = i + 1;
            long ts = now - (i * 3600);
            int group = new[] { 0x00, 0x01, 0x02, 0x03, 0x04 }[i % 5];
            int code = (i % 4) + 1;
            events.Add($"{eventId:X4}{ts:X8}{group:X2}{code:X2}");
        }

        return string.Join("-", events);
    }

    private static string BuildTelemetryCsv() {
        DateTime utc = DateTime.UtcNow;
        uint timestamp = (uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        double battery = 3.95 + _random.NextDouble() * 0.15;
        double system = 3.30 + _random.NextDouble() * 0.05;
        double usb = 120 + _random.NextDouble() * 40;
        double solar = 80 + _random.NextDouble() * 60;
        double discharge = 90 + _random.NextDouble() * 30;

        string lat = "5213.4567";
        string lon = "02100.1234";
        string time = utc.ToString("HHmmss");
        string date = utc.ToString("ddMMyy");

        return string.Join(",",
            timestamp.ToString(CultureInfo.InvariantCulture),
            "10402",
            battery.ToString("F2", CultureInfo.InvariantCulture),
            system.ToString("F2", CultureInfo.InvariantCulture),
            usb.ToString("F2", CultureInfo.InvariantCulture),
            solar.ToString("F2", CultureInfo.InvariantCulture),
            discharge.ToString("F2", CultureInfo.InvariantCulture),
            time,
            lat,
            "N",
            lon,
            "E",
            "0.15",
            "45.0",
            date,
            "1",
            "9",
            "142.5");
    }

    private static string BuildSensorCsv() {
        uint timestamp = (uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        double temp = 21.0 + _random.NextDouble() * 4.0;
        double pressure = 1008.0 + _random.NextDouble() * 10.0;
        double humidity = 40.0 + _random.NextDouble() * 20.0;
        double light = 200.0 + _random.NextDouble() * 800.0;
        double ax = 0.02 + (_random.NextDouble() - 0.5) * 0.04;
        double ay = -0.01 + (_random.NextDouble() - 0.5) * 0.04;
        double az = 0.98 + (_random.NextDouble() - 0.5) * 0.04;
        double gx = (_random.NextDouble() - 0.5) * 0.4;
        double gy = (_random.NextDouble() - 0.5) * 0.4;
        double gz = (_random.NextDouble() - 0.5) * 0.2;

        return string.Join(",",
            timestamp.ToString(CultureInfo.InvariantCulture),
            temp.ToString("F1", CultureInfo.InvariantCulture),
            pressure.ToString("F1", CultureInfo.InvariantCulture),
            humidity.ToString("F1", CultureInfo.InvariantCulture),
            light.ToString("F1", CultureInfo.InvariantCulture),
            ax.ToString("F3", CultureInfo.InvariantCulture),
            ay.ToString("F3", CultureInfo.InvariantCulture),
            az.ToString("F3", CultureInfo.InvariantCulture),
            gx.ToString("F3", CultureInfo.InvariantCulture),
            gy.ToString("F3", CultureInfo.InvariantCulture),
            gz.ToString("F3", CultureInfo.InvariantCulture));
    }
}
