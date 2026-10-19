using System.Collections.Concurrent;
using System.Diagnostics;
using System.Globalization;
using System.Text;

namespace kubisat_operator;

/// <summary>
/// Manages command execution and response handling for satellite communication.
/// Frames use SEQ + CRC-16/CCITT-FALSE per software/commands.md.
/// </summary>
public class CommandManager {
    private bool _radioDebugEnabled;
    private ushort _nextSeq = 1;
    private readonly object _seqLock = new();

    public bool RadioDebugEnabled {
        get => _radioDebugEnabled;
        set => _radioDebugEnabled = value;
    }

    /// <summary>
    /// Raised when an unsolicited health beacon (VAL 1.4) is received.
    /// </summary>
    public event EventHandler<string>? HealthBeaconReceived;

    private const string FRAME_BEGIN = "KBST";
    private const string FRAME_END = "TSBK";
    private const char DELIMITER = ';';
    private const int MaxAttempts = 3;
    private static readonly TimeSpan CommandTimeout = TimeSpan.FromSeconds(8);

    private readonly SerialCommunicator _serialCommunicator;
    private readonly ConcurrentDictionary<string, CommandTask> _pendingCommands = new();
    private readonly ConcurrentDictionary<string, List<string>> _sequenceData = new();
    private readonly ConcurrentDictionary<ushort, string> _seqToCommandKey = new();

    public CommandManager(SerialCommunicator serialCommunicator) {
        _serialCommunicator = serialCommunicator;
        _serialCommunicator.FrameReceived += OnFrameReceived;
    }

    public Task<string> ExecuteCommandAsync(CommandDefinition command, string parameter = "", string operationType = "GET") {
        return ExecuteCommandAsync(command.GroupId, command.Id, parameter, operationType);
    }

    public async Task<string> ExecuteCommandAsync(byte groupId, byte commandId, string parameter = "", string operationType = "GET") {
        if (!_serialCommunicator.IsConnected)
            throw new InvalidOperationException("Port szeregowy nie jest otwarty");

        string commandKey = $"{groupId}-{commandId}-{operationType}";
        var completion = new TaskCompletionSource<string>(TaskCreationOptions.RunContinuationsAsynchronously);
        var commandTask = new CommandTask(completion, CancellationToken.None);

        if (!_pendingCommands.TryAdd(commandKey, commandTask))
            throw new InvalidOperationException($"Polecenie {commandKey} jest już w trakcie wykonywania");

        try {
            Exception? lastError = null;
            for (int attempt = 0; attempt < MaxAttempts; attempt++) {
                ushort seq = AllocateSeq();
                _seqToCommandKey[seq] = commandKey;
                commandTask.ActiveSeq = seq;

                if (_radioDebugEnabled)
                    await _serialCommunicator.SendFrameAsync("?");

                string frame = CreateCommandFrame(groupId, commandId, parameter, operationType, seq, unit: "");
                await _serialCommunicator.SendFrameAsync(frame);

                try {
                    return await completion.Task.WaitAsync(CommandTimeout);
                }
                catch (TimeoutException ex) {
                    lastError = ex;
                    _seqToCommandKey.TryRemove(seq, out _);
                    // Recreate TCS for retry so a late reply cannot complete the next attempt incorrectly
                    if (attempt + 1 < MaxAttempts) {
                        completion = new TaskCompletionSource<string>(TaskCreationOptions.RunContinuationsAsynchronously);
                        commandTask.CompletionSource = completion;
                        commandTask.ActiveSeq = 0;
                    }
                }
            }

            _pendingCommands.TryRemove(commandKey, out _);
            throw new TimeoutException($"Przekroczono limit czasu dla polecenia {commandKey}", lastError);
        }
        catch (TimeoutException) {
            _pendingCommands.TryRemove(commandKey, out _);
            throw;
        }
        catch (Exception ex) when (ex is not TimeoutException and not InvalidOperationException) {
            _pendingCommands.TryRemove(commandKey, out _);
            throw new Exception($"Błąd wykonywania polecenia {commandKey}: {ex.Message}", ex);
        }
    }

    private ushort AllocateSeq() {
        lock (_seqLock) {
            ushort seq = _nextSeq++;
            if (_nextSeq == 0)
                _nextSeq = 1;
            return seq;
        }
    }

    /// <summary>
    /// Builds a full integrity frame: KBST;ADDR;TYPE;GROUP;CMD;PAYLOAD;UNIT;SEQ;CRC16;TSBK
    /// </summary>
    public static string CreateCommandFrame(byte groupId, byte commandId, string parameter, string operationType, ushort seq, string unit = "") {
        string crcPayload = $"0{DELIMITER}{operationType}{DELIMITER}{groupId}{DELIMITER}{commandId}{DELIMITER}{parameter}{DELIMITER}{unit}{DELIMITER}{seq}";
        string crcHex = Crc16.FormatHex(Crc16.ComputeAscii(crcPayload));
        return $"{FRAME_BEGIN}{DELIMITER}{crcPayload}{DELIMITER}{crcHex}{DELIMITER}{FRAME_END}";
    }

    /// <summary>
    /// Builds a host ACK frame for an unsolicited sat frame (direction 0, same SEQ).
    /// </summary>
    public static string CreateAckFrame(byte groupId, byte commandId, ushort seq) {
        string crcPayload = $"0{DELIMITER}ACK{DELIMITER}{groupId}{DELIMITER}{commandId}{DELIMITER}{seq}{DELIMITER}{DELIMITER}{seq}";
        string crcHex = Crc16.FormatHex(Crc16.ComputeAscii(crcPayload));
        return $"{FRAME_BEGIN}{DELIMITER}{crcPayload}{DELIMITER}{crcHex}{DELIMITER}{FRAME_END}";
    }

    private void OnFrameReceived(object? sender, FrameReceivedEventArgs e) {
        try {
            ProcessFrame(e.Frame);
        }
        catch (Exception ex) {
            Debug.WriteLine($"Error processing frame: {ex.Message}");
        }
    }

    private void ProcessFrame(string frame) {
        // Split protocol frame from optional ;rssi;snr after TSBK
        string protocol = frame;
        string? rssiExtra = null;
        int endIdx = frame.IndexOf(FRAME_END, StringComparison.Ordinal);
        if (endIdx >= 0) {
            int after = endIdx + FRAME_END.Length;
            if (after < frame.Length)
                rssiExtra = frame.Substring(after);
            protocol = frame.Substring(0, after);
        }

        string[] parts = protocol.Split(DELIMITER);
        if (parts.Length < 6 || parts[0] != FRAME_BEGIN)
            return;

        if (parts[1] != "1")
            return;

        string responseType = parts[2];
        if (!int.TryParse(parts[3], out int groupId) || !int.TryParse(parts[4], out int commandId))
            return;

        string value = parts.Length > 5 ? parts[5] : "";
        string unit = "";
        ushort seq = 0;
        bool hasIntegrity = false;

        // New format: KBST;1;TYPE;G;C;PAYLOAD;UNIT;SEQ;CRC;TSBK  (10 tokens)
        // Legacy:     KBST;1;TYPE;G;C;PAYLOAD;TSBK                 (7 tokens)
        // Legacy+unit:KBST;1;TYPE;G;C;PAYLOAD;UNIT;TSBK            (8 tokens)
        if (parts.Length >= 10 && parts[^1] == FRAME_END) {
            unit = parts[6];
            if (ushort.TryParse(parts[7], out seq))
                hasIntegrity = true;
            // Verify CRC when present
            if (hasIntegrity && parts.Length >= 10) {
                string crcPayload = $"{parts[1]}{DELIMITER}{parts[2]}{DELIMITER}{parts[3]}{DELIMITER}{parts[4]}{DELIMITER}{parts[5]}{DELIMITER}{parts[6]}{DELIMITER}{parts[7]}";
                if (ushort.TryParse(parts[8], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out ushort gotCrc)) {
                    ushort expect = Crc16.ComputeAscii(crcPayload);
                    if (gotCrc != expect) {
                        Debug.WriteLine($"CRC mismatch: got {parts[8]} expect {Crc16.FormatHex(expect)}");
                        return;
                    }
                }
            }
        }
        else if (parts.Length >= 7 && parts[^1] == FRAME_END) {
            // Legacy: optional unit before TSBK
            if (parts.Length >= 8 && parts[6] != FRAME_END)
                unit = parts[6];
        }

        if (!string.IsNullOrEmpty(rssiExtra)) {
            // ";rssi;snr" or "rssi;snr"
            string extra = rssiExtra.TrimStart(';');
            string[] rssiParts = extra.Split(';');
            if (rssiParts.Length >= 2 &&
                int.TryParse(rssiParts[0], out int rssi) &&
                float.TryParse(rssiParts[1], CultureInfo.InvariantCulture, out float snr)) {
                OnRadioDebugDataReceived?.Invoke(this, new RadioDebugEventArgs(rssi, snr));
            }
        }

        // Ignore ACK frames from sat (command completion waits for VAL/RES/SEQ/ERR)
        if (responseType == "ACK")
            return;

        // Auto-ACK unsolicited health beacons so sat stops retrying
        bool isHealthBeacon = responseType == "VAL" && groupId == 1 && commandId == 4;
        if (isHealthBeacon && hasIntegrity) {
            string pendingKey = $"{groupId}-{commandId}-GET";
            bool isSolicited = _pendingCommands.ContainsKey(pendingKey);
            if (!isSolicited) {
                _ = SendBeaconAckAsync((byte)groupId, (byte)commandId, seq);
                HealthBeaconReceived?.Invoke(this, value);
                return;
            }
        }

        string getKey = $"{groupId}-{commandId}-GET";
        string setKey = $"{groupId}-{commandId}-SET";

        // Prefer SEQ correlation when available
        string? correlatedKey = null;
        if (hasIntegrity && _seqToCommandKey.TryGetValue(seq, out string? seqKey))
            correlatedKey = seqKey;

        switch (responseType) {
            case "VAL":
                HandleValueResponse(correlatedKey ?? getKey, value, seq);
                break;
            case "SEQ":
                HandleSequenceResponse(correlatedKey ?? getKey, value);
                break;
            case "RES":
                CompletePendingCommand(correlatedKey ?? setKey, value, seq);
                break;
            case "ERR":
                HandleErrorResponse(correlatedKey ?? getKey, correlatedKey ?? setKey, value, seq);
                break;
        }
    }

    private async Task SendBeaconAckAsync(byte groupId, byte commandId, ushort seq) {
        try {
            string ack = CreateAckFrame(groupId, commandId, seq);
            await _serialCommunicator.SendFrameAsync(ack);
        }
        catch (Exception ex) {
            Debug.WriteLine($"Failed to ACK health beacon: {ex.Message}");
        }
    }

    public event EventHandler<RadioDebugEventArgs>? OnRadioDebugDataReceived;

    public class RadioDebugEventArgs : EventArgs {
        public int Rssi { get; }
        public float Snr { get; }

        public RadioDebugEventArgs(int rssi, float snr) {
            Rssi = rssi;
            Snr = snr;
        }
    }

    private void HandleValueResponse(string commandKey, string value, ushort seq) {
        if (value == "SEQ_DONE") {
            if (_sequenceData.TryGetValue(commandKey, out List<string>? data)) {
                string accumulatedData = string.Join(";", data);
                CompletePendingCommand(commandKey, accumulatedData, seq);
                _sequenceData.TryRemove(commandKey, out _);
            }
            return;
        }

        CompletePendingCommand(commandKey, value, seq);
    }

    private void HandleSequenceResponse(string commandKey, string value) {
        if (!_sequenceData.TryGetValue(commandKey, out List<string>? data)) {
            data = new List<string>();
            _sequenceData[commandKey] = data;
        }
        data.Add(value);
    }

    private void HandleErrorResponse(string getKey, string setKey, string errorMessage, ushort seq) {
        if (errorMessage == "NO_DATA") {
            CompletePendingCommand(getKey, string.Empty, seq);
            return;
        }

        Exception exception = new CommandException(errorMessage);
        if (!FailPendingCommand(getKey, exception, seq))
            FailPendingCommand(setKey, exception, seq);
    }

    private bool FailPendingCommand(string key, Exception exception, ushort seq) {
        if (_pendingCommands.TryGetValue(key, out CommandTask? task)) {
            task.CompletionSource.TrySetException(exception);
            _pendingCommands.TryRemove(key, out _);
            if (seq != 0)
                _seqToCommandKey.TryRemove(seq, out _);
            return true;
        }
        return false;
    }

    private bool CompletePendingCommand(string key, string result, ushort seq) {
        if (_pendingCommands.TryGetValue(key, out CommandTask? task)) {
            task.CompletionSource.TrySetResult(result);
            _pendingCommands.TryRemove(key, out _);
            if (seq != 0)
                _seqToCommandKey.TryRemove(seq, out _);
            // Also clear any SEQ mapping for this task's active seq
            if (task.ActiveSeq != 0)
                _seqToCommandKey.TryRemove(task.ActiveSeq, out _);
            return true;
        }
        return false;
    }

    private class CommandTask {
        public TaskCompletionSource<string> CompletionSource { get; set; }
        public CancellationToken CancellationToken { get; }
        public ushort ActiveSeq { get; set; }

        public CommandTask(TaskCompletionSource<string> completionSource, CancellationToken cancellationToken) {
            CompletionSource = completionSource;
            CancellationToken = cancellationToken;
        }
    }
}

public class CommandException : Exception {
    public CommandException(string message) : base(message) { }
}
