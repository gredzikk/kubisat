using System.Collections.Concurrent;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.IO.Ports;
using System.Text;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;
using OxyPlot;
using OxyPlot.Series;
using static kubisat_operator.CommandManager;

namespace kubisat_operator;

public class TestCommandResult {
    public int Index { get; set; }
    public string Operation { get; set; } = string.Empty;
    public string Parameter { get; set; } = string.Empty;
    public string Status { get; set; } = string.Empty;
    public double ExecutionTime { get; set; }
    public string Response { get; set; } = string.Empty;
}

public class AccessTestResult {
    public string CommandName { get; set; } = string.Empty;
    public string CommandId { get; set; } = string.Empty;
    public string AccessType { get; set; } = string.Empty;
    public string GetAllowed { get; set; } = string.Empty;
    public string GetStatus { get; set; } = string.Empty;
    public string SetAllowed { get; set; } = string.Empty;
    public string SetStatus { get; set; } = string.Empty;
    public string Result { get; set; } = string.Empty;
}

public partial class MainWindow : Window {
    private const string FRAME_BEGIN = "KBST";
    private const string FRAME_END = "TSBK";
    private const char DELIMITER = ';';
    private PlotModel? _mapPlotModel;
    private ScatterSeries? _positionSeries;
    private LineSeries? _trackingSeries;

    private SerialPort? serialPort;
    private ConcurrentDictionary<string, string> receivedResponses = new();
    private TaskCompletionSource<bool>? allCommandsReceived;

    private Dictionary<string, List<string>> multiFrameData = new Dictionary<string, List<string>>();
    private TelemetryPlotManager _telemetryPlotManager = null!;

    private SerialCommunicator _serialCommunicator = null!;
    private CommandManager _commandManager = null!;
    private ObservableCollection<TestCommandResult> _batchTestResults = new ObservableCollection<TestCommandResult>();
    private ObservableCollection<AccessTestResult> _accessTestResults = new ObservableCollection<AccessTestResult>();
    private CommandDefinition? _currentTestCommand;
    private bool _isBatchTestRunning = false;
    private CancellationTokenSource? _batchTestCancellation;

    private DispatcherTimer? _portPollTimer;
    private ObservableCollection<SerialLogLine> _serialLogLines = new();

    public MainWindow() {
        InitializeComponent();
        CommandRegistry.Initialize();

        _serialCommunicator = new SerialCommunicator();
        _serialCommunicator.RawDataReceived += OnRawDataReceived;

        _commandManager = new CommandManager(_serialCommunicator);
        _commandManager.OnRadioDebugDataReceived += OnRadioDebugDataReceived;
        _commandManager.HealthBeaconReceived += OnHealthBeaconReceived;

        DataContext = this;
        SerialMonitorItems.ItemsSource = _serialLogLines;

        PopulatePortList();
        StartDeviceWatcher();
        serialPort = new SerialPort();

        _telemetryPlotManager = new TelemetryPlotManager();
        VoltagePlot.Model = _telemetryPlotManager.VoltagePlotModel;
        CurrentPlot.Model = _telemetryPlotManager.CurrentPlotModel;

        _sensorPlotManager = new SensorPlotManager();
        EnvironmentPlot.Model = _sensorPlotManager.EnvironmentPlotModel;
        LightPlot.Model = _sensorPlotManager.LightPlotModel;
        PressurePlot.Model = _sensorPlotManager.PressurePlotModel;

        InitializeGpsVisualization();
        InitializeTestMode();

        RadioDebugCheckBox.IsCheckedChanged += RadioDebugCheckBox_CheckedChanged;
    }

    private void OnHealthBeaconReceived(object? sender, string payload) {
        Dispatcher.UIThread.Post(() => {
            HealthBeaconLabel.Text = $"Beacon: {payload}";
        });
    }

    /// <summary>
    /// Starts monitoring for serial port changes via polling.
    /// </summary>
    private void StartDeviceWatcher() {
        _portPollTimer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(2) };
        _portPollTimer.Tick += (_, _) => PopulatePortList();
        _portPollTimer.Start();
    }

    /// <summary>
    /// Populates the COM port combo box with available serial ports.
    /// Preserves the current selection if the port is still available.
    /// </summary>
    private void PopulatePortList() {
        string? selectedPort = PortComboBox.SelectedItem as string;
        string[] ports = SerialPort.GetPortNames()
            .Concat(new[] { DemoSimulator.PortName })
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .OrderBy(p => p.Equals(DemoSimulator.PortName, StringComparison.OrdinalIgnoreCase) ? 0 : 1)
            .ThenBy(p => p, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        PortComboBox.ItemsSource = ports;

        if (ports.Length > 0) {
            if (selectedPort != null && ports.Contains(selectedPort, StringComparer.OrdinalIgnoreCase))
                PortComboBox.SelectedItem = ports.First(p => p.Equals(selectedPort, StringComparison.OrdinalIgnoreCase));
            else
                PortComboBox.SelectedIndex = 0;
        }
        else {
            PortComboBox.SelectedItem = null;
        }
    }

    private void ConnectButton_Click(object? sender, RoutedEventArgs e) {
        if (_serialCommunicator.IsConnected) {
            Disconnect();
        }
        else {
            Connect();
        }
    }

    private void OnRawDataReceived(object? sender, string data) {
        Dispatcher.UIThread.Post(() => {
            string displayLine = _showTimestamps
                ? $"[{DateTime.Now:HH:mm:ss.fff}] - {data}"
                : data;

            if (data.Contains("KBST;1;")) {
                if (data.Contains(";ERR;"))
                    AppendColoredText(displayLine, "Red");
                else
                    AppendColoredText(displayLine, "Blue");
            }
            else if (data.Contains("KBST;0;"))
                AppendColoredText(displayLine, "Green");
            else
                AppendColoredText(displayLine, "Black");
        });
    }

    private void Connect() {
        if (PortComboBox.SelectedItem == null) {
            DialogHelper.ShowMessage(this, "Wybierz port szeregowy.");
            return;
        }

        try {
            string portName = PortComboBox.SelectedItem.ToString()!;
            int baudRate = 115200;

            _serialCommunicator.Connect(portName, baudRate);

            MainTabControl.IsVisible = true;
            ConnectButton.Content = "Rozłącz";

            if (_serialCommunicator.IsDemoMode)
                AppendColoredText("Połączono z portem demo (symulowane urządzenie, bez sprzętu)", "Green");
            else
                AppendColoredText($"Połączono z {portName} przy {baudRate} baud", "Green");
        }
        catch (Exception ex) {
            DialogHelper.ShowMessage(this, $"Błąd połączenia: {ex.Message}", "Błąd");
        }
    }

    private void Disconnect() {
        _serialCommunicator.Disconnect();
        MainTabControl.IsVisible = false;
        ConnectButton.Content = "Połącz";
    }

    private void RadioDebugCheckBox_CheckedChanged(object? sender, RoutedEventArgs e) {
        if (_commandManager != null)
            _commandManager.RadioDebugEnabled = RadioDebugCheckBox.IsChecked ?? false;
    }

    private void OnRadioDebugDataReceived(object? sender, RadioDebugEventArgs e) {
        Dispatcher.UIThread.Post(() => {
            RssiValueLabel.Content = $"{e.Rssi} dBm";
            SnrValueLabel.Content = $"{e.Snr:F1} dB";

            RssiValueLabel.Foreground = e.Rssi > -60 ? Brushes.Green :
                                       e.Rssi > -90 ? Brushes.Orange :
                                       Brushes.Red;

            SnrValueLabel.Foreground = e.Snr > 10 ? Brushes.Green :
                                      e.Snr > 5 ? Brushes.Orange :
                                      Brushes.Red;
        });
    }

    private DateTime _lastFrameSentTime;

    private async void ImportTelemetryDataButton_Click(object? sender, RoutedEventArgs e) {
        string? fileName = await DialogHelper.OpenFileAsync(this, "Wybierz plik CSV z danymi telemetrycznymi", DialogHelper.CsvFileType);
        if (fileName == null) return;

        try {
            _telemetryPlotManager.LoadFromCsv(fileName);
            _positionHistory.Clear();

            foreach (string line in File.ReadLines(fileName)) {
                try {
                    TelemetryRecord record = TelemetryRecord.FromCsv(line);

                    if (TryParseNmeaCoordinate(record.Latitude, record.LatDir, out double latitude) &&
                        TryParseNmeaCoordinate(record.Longitude, record.LonDir, out double longitude) &&
                        double.TryParse(record.Altitude, NumberStyles.Any, CultureInfo.InvariantCulture, out double altitude)) {
                        DateTime timestamp = DateTime.TryParseExact($"{record.Time} {record.Date}", "HHmmss ddMMyy",
                            CultureInfo.InvariantCulture, DateTimeStyles.None, out DateTime gpsDateTime)
                            ? gpsDateTime
                            : DateTimeOffset.FromUnixTimeSeconds(record.Timestamp).DateTime;
                        GpsPosition position = new GpsPosition {
                            Timestamp = timestamp,
                            Latitude = latitude,
                            Longitude = longitude,
                            Altitude = altitude,
                            Satellites = record.Satellites
                        };

                        _positionHistory.Add(position);
                    }
                }
                catch (Exception ex) {
                    Debug.WriteLine($"Error processing line: {ex.Message}");
                }
            }

            string? lastLine = File.ReadLines(fileName).LastOrDefault();
            if (lastLine != null) {
                TelemetryRecord record = TelemetryRecord.FromCsv(lastLine);
                UpdateTelemetryTable(record);
                UpdateGpsVisualization(record);
            }

            DialogHelper.ShowMessage(this, $"Zaimportowano {_positionHistory.Count} rekordów pozycji GPS.", "Sukces");
        }
        catch (Exception ex) {
            DialogHelper.ShowMessage(this, $"Błąd podczas importowania danych: {ex.Message}", "Błąd");
        }
    }

    private async void ImportSensorDataButton_Click(object? sender, RoutedEventArgs e) {
        string? fileName = await DialogHelper.OpenFileAsync(this, "Wybierz plik CSV z danymi z czujników", DialogHelper.CsvFileType);
        if (fileName == null) return;

        try {
            _sensorPlotManager.LoadFromCsv(fileName);

            string? lastLine = File.ReadLines(fileName).LastOrDefault();
            if (lastLine != null) {
                SensorDataRecord record = SensorDataRecord.FromCsv(lastLine);
                UpdateSensorTable(record);
            }

            DialogHelper.ShowMessage(this, "Dane zostały zaimportowane pomyślnie.", "Sukces");
        }
        catch (Exception ex) {
            DialogHelper.ShowMessage(this, $"Błąd podczas importowania danych: {ex.Message}", "Błąd");
        }
    }

    private void InitializeTestMode() {
        TestCommandGroupComboBox.ItemsSource = CommandRegistry.Groups;

        BatchTestResultsDataGrid.ItemsSource = _batchTestResults;

        TestCommandGetButton.IsEnabled = false;
        TestCommandSetButton.IsEnabled = false;
    }

    private void TestCommandGroupComboBox_SelectionChanged(object? sender, SelectionChangedEventArgs e) {
        if (TestCommandGroupComboBox.SelectedItem is CommandGroup group) {
            TestCommandComboBox.ItemsSource = group.Commands;
            TestCommandComboBox.SelectedIndex = -1;

            TestCommandDetailsText.Text = string.Empty;
            TestCommandAccessText.Text = string.Empty;
            _currentTestCommand = null;

            TestCommandGetButton.IsEnabled = false;
            TestCommandSetButton.IsEnabled = false;
        }
    }

    private void TestCommandComboBox_SelectionChanged(object? sender, SelectionChangedEventArgs e) {
        if (TestCommandComboBox.SelectedItem is CommandDefinition command) {
            _currentTestCommand = command;

            TestCommandDetailsText.Text = $"ID: {command.GroupId}.{command.Id} - {command.Description}";
            TestCommandAccessText.Text = $"Uprawnienia: {GetAccessTypeDescription(command.Access)}";
            TestCommandGetButton.IsEnabled = true;
            TestCommandSetButton.IsEnabled = true;
        }
    }

    private string GetAccessTypeDescription(CommandAccessType accessType) {
        switch (accessType) {
            case CommandAccessType.ReadOnly:
                return "Tylko do odczytu (GET)";
            case CommandAccessType.WriteOnly:
                return "Tylko do zapisu (SET)";
            case CommandAccessType.ReadWrite:
                return "Odczyt i zapis (GET/SET)";
            default:
                return accessType.ToString();
        }
    }

    private async void TestCommandGetButton_Click(object? sender, RoutedEventArgs e) {
        await ExecuteTestCommand("GET");
    }

    private async void TestCommandSetButton_Click(object? sender, RoutedEventArgs e) {
        await ExecuteTestCommand("SET");
    }

    private async Task ExecuteTestCommand(string operationType) {
        if (_currentTestCommand == null) return;

        TestStatusTextBlock.Text = "Wykonywanie...";
        TestResponseTextBox.Text = string.Empty;
        TestExecutionTimeTextBlock.Text = string.Empty;

        string parameter = TestCommandParameterTextBox.Text?.Trim() ?? string.Empty;

        try {
            Stopwatch stopwatch = Stopwatch.StartNew();

            string response = await _commandManager.ExecuteCommandAsync(
                _currentTestCommand,
                parameter,
                operationType);

            stopwatch.Stop();

            TestStatusTextBlock.Text = "Sukces";
            TestStatusTextBlock.Foreground = Brushes.Green;
            TestResponseTextBox.Text = response;
            TestExecutionTimeTextBlock.Text = $"{stopwatch.ElapsedMilliseconds} ms";

            TestLogListBox.Items.Add($"[{DateTime.Now:HH:mm:ss.fff}] {_currentTestCommand.Name} ({operationType}): Sukces, czas: {stopwatch.ElapsedMilliseconds} ms");
        }
        catch (Exception ex) {
            TestStatusTextBlock.Text = "Błąd";
            TestStatusTextBlock.Foreground = Brushes.Red;
            TestResponseTextBox.Text = ex.Message;
            TestExecutionTimeTextBlock.Text = "—";

            TestLogListBox.Items.Add($"[{DateTime.Now:HH:mm:ss.fff}] {_currentTestCommand.Name} ({operationType}): Błąd - {ex.Message}");
        }

        if (TestLogListBox.Items.Count > 0)
            TestLogListBox.ScrollIntoView(TestLogListBox.Items[TestLogListBox.Items.Count - 1]!);
    }

    private void ClearTestLogButton_Click(object? sender, RoutedEventArgs e) {
        TestLogListBox.Items.Clear();
    }

    private async void RunBatchTestButton_Click(object? sender, RoutedEventArgs e) {
        if (_currentTestCommand == null) return;

        if (_isBatchTestRunning) {
            _batchTestCancellation?.Cancel();
            return;
        }

        int repetitions = (int)(BatchTestRepetitionsUpDown.Value ?? 10);
        int delay = (int)(BatchTestDelayUpDown.Value ?? 100);

        _batchTestResults.Clear();
        BatchTestAvgTimeTextBlock.Text = string.Empty;
        BatchTestSuccessRateTextBlock.Text = string.Empty;
        BatchTestStatusTextBlock.Text = "Uruchamianie testu...";

        _batchTestCancellation = new CancellationTokenSource();
        _isBatchTestRunning = true;

        try {
            BatchTestProgressBar.Value = 0;
            BatchTestProgressBar.Maximum = repetitions;
            RunBatchTestButton.Content = "Anuluj test";

            List<string> operations = new List<string>();
            if (_currentTestCommand.Access == CommandAccessType.ReadOnly ||
                _currentTestCommand.Access == CommandAccessType.ReadWrite)
                operations.Add("GET");

            if (_currentTestCommand.Access == CommandAccessType.WriteOnly ||
                _currentTestCommand.Access == CommandAccessType.ReadWrite)
                operations.Add("SET");

            string parameter = TestCommandParameterTextBox.Text?.Trim() ?? string.Empty;

            int successCount = 0;
            double totalTime = 0;

            for (int i = 0; i < repetitions; i++) {
                if (_batchTestCancellation.Token.IsCancellationRequested)
                    break;

                string operation = operations.Count == 1 ? operations[0] : operations[i % operations.Count];

                try {
                    BatchTestStatusTextBlock.Text = $"Wykonywanie testu {i + 1}/{repetitions} ({operation})...";

                    Stopwatch stopwatch = Stopwatch.StartNew();
                    string response = await _commandManager.ExecuteCommandAsync(
                        _currentTestCommand,
                        parameter,
                        operation);
                    stopwatch.Stop();

                    _batchTestResults.Add(new TestCommandResult {
                        Index = i + 1,
                        Operation = operation,
                        Parameter = parameter,
                        Status = "Sukces",
                        ExecutionTime = stopwatch.ElapsedMilliseconds,
                        Response = response
                    });

                    successCount++;
                    totalTime += stopwatch.ElapsedMilliseconds;

                    if (delay > 0 && i < repetitions - 1)
                        await Task.Delay(delay, _batchTestCancellation.Token);
                }
                catch (TaskCanceledException) {
                    break;
                }
                catch (Exception ex) {
                    _batchTestResults.Add(new TestCommandResult {
                        Index = i + 1,
                        Operation = operation,
                        Parameter = parameter,
                        Status = "Błąd",
                        ExecutionTime = 0,
                        Response = ex.Message
                    });
                }

                BatchTestProgressBar.Value = i + 1;
            }

            if (_batchTestResults.Count > 0 && successCount > 0) {
                double avgTime = totalTime / successCount;
                double successRate = (double)successCount / _batchTestResults.Count * 100;

                BatchTestAvgTimeTextBlock.Text = $"{avgTime:F2} ms";
                BatchTestSuccessRateTextBlock.Text = $"{successRate:F1}% ({successCount}/{_batchTestResults.Count})";
            }

            BatchTestStatusTextBlock.Text = _batchTestCancellation.Token.IsCancellationRequested ?
                "Test został anulowany" : "Test zakończony";
        }
        catch (Exception ex) {
            BatchTestStatusTextBlock.Text = $"Błąd testu: {ex.Message}";
        }
        finally {
            _isBatchTestRunning = false;
            RunBatchTestButton.Content = "Uruchom test wsadowy";
            _batchTestCancellation = null;
        }
    }

    private async void ExportBatchResultsButton_Click(object? sender, RoutedEventArgs e) {
        if (_batchTestResults.Count == 0) {
            DialogHelper.ShowMessage(this, "Brak danych do eksportu.", "Informacja");
            return;
        }

        string? fileName = await DialogHelper.SaveFileAsync(this, "Eksportuj wyniki testu wsadowego do CSV", "batch_results.csv", DialogHelper.CsvFileType);
        if (fileName == null) return;

        try {
            using (StreamWriter writer = new StreamWriter(fileName, false, Encoding.UTF8)) {
                writer.WriteLine("Indeks,Operacja,Parametr,Status,Czas [ms],Odpowiedź");

                foreach (var result in _batchTestResults) {
                    writer.WriteLine($"{result.Index},{result.Operation},{result.Parameter},{result.Status},{result.ExecutionTime},\"{result.Response?.Replace("\"", "\"\"")}\"");
                }
            }

            DialogHelper.ShowMessage(this, $"Dane zostały wyeksportowane do pliku: {fileName}", "Sukces");
        }
        catch (Exception ex) {
            DialogHelper.ShowMessage(this, $"Błąd podczas eksportowania danych: {ex.Message}", "Błąd");
        }
    }
}
