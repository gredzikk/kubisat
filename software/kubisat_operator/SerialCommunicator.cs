using System.Diagnostics;
using System.IO.Ports;
using System.Text;

namespace kubisat_operator;

public class SerialCommunicator {
    private SerialPort? _serialPort;
    private StringBuilder _buffer = new StringBuilder();
    private const string FRAME_BEGIN = "KBST";
    private const string FRAME_END = "TSBK";
    private bool _demoMode;

    public event EventHandler<FrameReceivedEventArgs>? FrameReceived;
    public event EventHandler<string>? RawDataReceived;

    public bool IsDemoMode => _demoMode;
    public bool IsConnected => _demoMode || (_serialPort?.IsOpen ?? false);

    public bool Connect(string portName, int baudRate) {
        try {
            Disconnect();

            if (string.Equals(portName, DemoSimulator.PortName, StringComparison.OrdinalIgnoreCase)) {
                _demoMode = true;
                return true;
            }

            _demoMode = false;
            _serialPort = new SerialPort {
                PortName = portName,
                BaudRate = baudRate,
                DataBits = 8,
                StopBits = StopBits.One,
                Parity = Parity.None,
                Handshake = Handshake.None,
                DtrEnable = true,
                RtsEnable = true,
                WriteTimeout = 1000,
                ReadBufferSize = 4096,
                WriteBufferSize = 4096,
                NewLine = "\n"
            };

            _serialPort.DataReceived += OnDataReceived;
            _serialPort.Open();

            _serialPort.DiscardInBuffer();
            _serialPort.DiscardOutBuffer();

            return true;
        }
        catch (Exception) {
            Disconnect();
            throw;
        }
    }

    public void Disconnect() {
        _demoMode = false;

        if (_serialPort == null) return;

        if (_serialPort.IsOpen) {
            _serialPort.DataReceived -= OnDataReceived;
            _serialPort.Close();
        }
        _serialPort.Dispose();
        _serialPort = null;
        _buffer.Clear();
    }

    public Task SendFrameAsync(string frame) {
        if (!IsConnected)
            throw new InvalidOperationException("Serial port is not open");

        if (_demoMode)
            return SendDemoFrameAsync(frame);

        return Task.Run(() => {
            byte[] frameBytes = Encoding.ASCII.GetBytes(frame + "\n");
            _serialPort!.Write(frameBytes, 0, frameBytes.Length);
            _serialPort.BaseStream.Flush();
        });
    }

    private async Task SendDemoFrameAsync(string frame) {
        RawDataReceived?.Invoke(this, frame);
        await Task.Delay(40);

        string? response = DemoSimulator.BuildResponse(frame);
        if (response == null)
            return;

        RawDataReceived?.Invoke(this, response);
        FrameReceived?.Invoke(this, new FrameReceivedEventArgs(response));
    }

    private void OnDataReceived(object sender, SerialDataReceivedEventArgs e) {
        try {
            string data = _serialPort!.ReadExisting();
            _buffer.Append(data);

            string bufferContent = _buffer.ToString();
            int lineEndIndex;

            while ((lineEndIndex = bufferContent.IndexOf('\n')) != -1) {
                string line = bufferContent.Substring(0, lineEndIndex).Trim();
                RawDataReceived?.Invoke(this, line);
                bufferContent = bufferContent.Substring(lineEndIndex + 1);

                int frameStartIndex = line.IndexOf(FRAME_BEGIN);
                if (frameStartIndex >= 0) {
                    string frameWithExtra = line.Substring(frameStartIndex);
                    int frameEndIndex = frameWithExtra.IndexOf(FRAME_END);
                    if (frameEndIndex >= 0) {
                        string frame = frameWithExtra.Substring(0, frameEndIndex + FRAME_END.Length);
                        string extraData = frameWithExtra.Length > frameEndIndex + FRAME_END.Length
                            ? frameWithExtra.Substring(frameEndIndex + FRAME_END.Length)
                            : "";

                        if (!string.IsNullOrEmpty(extraData)) {
                            frame += extraData;
                        }
                        FrameReceived?.Invoke(this, new FrameReceivedEventArgs(frame));
                    }
                }
            }

            _buffer = new StringBuilder(bufferContent);
        }
        catch (Exception ex) {
            Debug.WriteLine($"Error in data received: {ex.Message}");
        }
    }
}

public class FrameReceivedEventArgs : EventArgs {
    public string Frame { get; }

    public FrameReceivedEventArgs(string frame) {
        Frame = frame;
    }
}
