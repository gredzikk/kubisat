using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Legends;
using OxyPlot.Series;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;

namespace kubisat_operator {
    public class TelemetryRecord {
        // Timestamp
        public uint Timestamp { get; set; }

        public uint BuildVersion { get; set; }

        // Power data
        public float BatteryVoltage { get; set; }
        public float SystemVoltage { get; set; }
        public float ChargeCurrentUsb { get; set; }
        public float ChargeCurrentSolar { get; set; }
        public float DischargeCurrent { get; set; }

        // GPS RMC data
        public string Time { get; set; }
        public string Latitude { get; set; }
        public string LatDir { get; set; }
        public string Longitude { get; set; }
        public string LonDir { get; set; }
        public string Speed { get; set; }
        public string Course { get; set; }
        public string Date { get; set; }

        // GPS GGA data
        public string FixQuality { get; set; }
        public string Satellites { get; set; }
        public string Altitude { get; set; }

        public static TelemetryRecord FromCsv(string csvData) {
            string[] parts = csvData.Split(',');

            return parts.Length < 17
                ? throw new ArgumentException("Nieprawidłowy format danych telemetrycznych")
                : new TelemetryRecord {
                    Timestamp = uint.Parse(parts[0]),
                    BuildVersion = uint.Parse(parts[1]),
                    BatteryVoltage = float.Parse(parts[2], System.Globalization.CultureInfo.InvariantCulture),
                    SystemVoltage = float.Parse(parts[3], System.Globalization.CultureInfo.InvariantCulture),
                    ChargeCurrentUsb = float.Parse(parts[4], System.Globalization.CultureInfo.InvariantCulture),
                    ChargeCurrentSolar = float.Parse(parts[5], System.Globalization.CultureInfo.InvariantCulture),
                    DischargeCurrent = float.Parse(parts[6], System.Globalization.CultureInfo.InvariantCulture),
                    Time = parts[7],
                    Latitude = parts[8],
                    LatDir = parts[9],
                    Longitude = parts[10],
                    LonDir = parts[11],
                    Speed = parts[12],
                    Course = parts[13],
                    Date = parts[14],
                    FixQuality = parts[15],
                    Satellites = parts[16],
                    Altitude = parts[17]
                };
        }
    }

    public class TelemetryPlotManager {
        private PlotModel _voltagePlotModel;
        private PlotModel _currentPlotModel;
        private LineSeries _batteryVoltageSeries;
        private LineSeries _systemVoltageSeries;
        private LineSeries _chargeCurrentUsbSeries;
        private LineSeries _chargeCurrentSolarSeries;
        private LineSeries _dischargeCurrentSeries;
        private DateTimeAxis _voltageTimeAxis;
        private DateTimeAxis _currentTimeAxis;


        public PlotModel VoltagePlotModel => _voltagePlotModel;
        public PlotModel CurrentPlotModel => _currentPlotModel;

        public TelemetryPlotManager() {
            InitializeVoltagePlot();
            InitializeCurrentPlot();
        }

        private void InitializeVoltagePlot() {
            _voltagePlotModel = new PlotModel {
                Title = "Napięcia"
            };

            _voltagePlotModel.Legends.Add(new Legend {
                LegendPosition = LegendPosition.RightTop,
                LegendPlacement = LegendPlacement.Outside,
                LegendOrientation = LegendOrientation.Vertical,
                LegendBorder = OxyColors.Transparent 
            });

            _voltageTimeAxis = new DateTimeAxis {
                Position = AxisPosition.Bottom,
                Title = "Czas",
                StringFormat = "HH:mm:ss",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            LinearAxis voltageAxis = new LinearAxis {
                Position = AxisPosition.Left,
                Title = "Napięcie [V]",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            _voltagePlotModel.Axes.Add(_voltageTimeAxis);
            _voltagePlotModel.Axes.Add(voltageAxis);

            _batteryVoltageSeries = new LineSeries {
                Title = "Napięcie baterii",
                Color = OxyColor.FromRgb(0, 114, 189),
                StrokeThickness = 2
            };

            _systemVoltageSeries = new LineSeries {
                Title = "Napięcie systemowe",
                Color = OxyColor.FromRgb(217, 83, 25), 
                StrokeThickness = 2
            };

            _voltagePlotModel.Series.Add(_batteryVoltageSeries);
            _voltagePlotModel.Series.Add(_systemVoltageSeries);
        }

        private LineSeries _totalChargeCurrentSeries;   
        private LineSeries _currentBalanceSeries;
        private void InitializeCurrentPlot() {
            _currentPlotModel = new PlotModel {
                Title = "Prądy"
            };

            _currentPlotModel.Legends.Add(new Legend {
                LegendPosition = LegendPosition.RightTop,
                LegendPlacement = LegendPlacement.Outside,
                LegendOrientation = LegendOrientation.Vertical,
                LegendBorder = OxyColors.Transparent 
            });

            _currentTimeAxis = new DateTimeAxis {
                Position = AxisPosition.Bottom,
                Title = "Czas",
                StringFormat = "HH:mm:ss",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            LinearAxis currentAxis = new LinearAxis {
                Position = AxisPosition.Left,
                Title = "Prąd [mA]",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            _currentPlotModel.Axes.Add(_currentTimeAxis);
            _currentPlotModel.Axes.Add(currentAxis);

            _chargeCurrentUsbSeries = new LineSeries {
                Title = "Prąd ładowania USB",
                Color = OxyColor.FromRgb(119, 172, 48), 
                StrokeThickness = 2,
                LineStyle = LineStyle.DashDot,
            };

            _chargeCurrentSolarSeries = new LineSeries {
                Title = "Prąd ładowania solarnego",
                Color = OxyColor.FromRgb(237, 177, 32),
                StrokeThickness = 2,
                LineStyle = LineStyle.DashDot,
            };

            _dischargeCurrentSeries = new LineSeries {
                Title = "Prąd rozładowania",
                Color = OxyColor.FromRgb(126, 47, 142), 
                StrokeThickness = 2,
                LineStyle = LineStyle.Dash
            };

            _totalChargeCurrentSeries = new LineSeries {
                Title = "Suma prądów ładowania",
                Color = OxyColor.FromRgb(0, 114, 189), 
                StrokeThickness = 2,
                LineStyle = LineStyle.Dash
            };

            _currentBalanceSeries = new LineSeries {
                Title = "Bilans prądowy",
                Color = OxyColor.FromRgb(217, 83, 25), 
                StrokeThickness = 2,
            };

            _currentPlotModel.Series.Add(_chargeCurrentUsbSeries);
            _currentPlotModel.Series.Add(_chargeCurrentSolarSeries);
            _currentPlotModel.Series.Add(_dischargeCurrentSeries);
            _currentPlotModel.Series.Add(_totalChargeCurrentSeries);
            _currentPlotModel.Series.Add(_currentBalanceSeries);
        }



        public void AddDataPoint(TelemetryRecord record) {
            DateTime time = DateTimeOffset.FromUnixTimeSeconds(record.Timestamp).DateTime;
            double timeValue = DateTimeAxis.ToDouble(time);

            _batteryVoltageSeries.Points.Add(new DataPoint(timeValue, record.BatteryVoltage));
            _systemVoltageSeries.Points.Add(new DataPoint(timeValue, record.SystemVoltage));

            _chargeCurrentUsbSeries.Points.Add(new DataPoint(timeValue, record.ChargeCurrentUsb));
            _chargeCurrentSolarSeries.Points.Add(new DataPoint(timeValue, record.ChargeCurrentSolar));
            _dischargeCurrentSeries.Points.Add(new DataPoint(timeValue, record.DischargeCurrent));

            float totalChargeCurrent = record.ChargeCurrentUsb + record.ChargeCurrentSolar;
            _totalChargeCurrentSeries.Points.Add(new DataPoint(timeValue, totalChargeCurrent));

            float currentBalance = totalChargeCurrent - record.DischargeCurrent;
            _currentBalanceSeries.Points.Add(new DataPoint(timeValue, currentBalance));

            _voltagePlotModel.InvalidatePlot(true);
            _currentPlotModel.InvalidatePlot(true);
        }

        public void ClearData() {
            _batteryVoltageSeries.Points.Clear();
            _systemVoltageSeries.Points.Clear();
            _chargeCurrentUsbSeries.Points.Clear();
            _chargeCurrentSolarSeries.Points.Clear();
            _dischargeCurrentSeries.Points.Clear();
            _totalChargeCurrentSeries.Points.Clear();
            _currentBalanceSeries.Points.Clear();

            _voltagePlotModel.InvalidatePlot(true);
            _currentPlotModel.InvalidatePlot(true);
        }

        public void LoadFromCsv(string filePath) {
            ClearData();
            string[] lines = File.ReadAllLines(filePath);
            foreach (string line in lines) {
                try {
                    TelemetryRecord record = TelemetryRecord.FromCsv(line);
                    AddDataPoint(record);
                }
                catch (Exception ex) {
                    Debug.WriteLine($"Error parsing line: {line}, Error: {ex.Message}");
                }
            }
        }
    }

    public partial class MainWindow : Window {
        public ObservableCollection<KeyValuePair<string, string>> TelemetryData { get; set; } =
            new ObservableCollection<KeyValuePair<string, string>>();

        public ObservableCollection<CommandViewModel> TelemetryCommands { get; set; }

        private DispatcherTimer telemetryRefreshTimer;
        private DateTime telemetryRefreshEndTime;
        private double telemetryRefreshTotalDuration;
        private bool isAutoRefreshActive = false;

        private async void GetLastTelemetryRecordButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetLastRecord");
                string response = await _commandManager.ExecuteCommandAsync(command);

                ProcessTelemetryRecord(response);
            }
            catch (Exception ex) {
                Debug.WriteLine($"Error getting telemetry record: {ex.Message}");
                DialogHelper.ShowMessage(this, $"Błąd pobierania rekordu telemetrycznego: {ex.Message}");
            }
        }

        private void StartTelemetryAutoRefreshButton_Click(object sender, RoutedEventArgs e) {
            if (isAutoRefreshActive) {
                StopTelemetryAutoRefresh();
            }
            else {
                StartTelemetryAutoRefresh();
            }
        }

        private async void StartTelemetryAutoRefresh() {
            if (TelemetryRefreshInterval.Value.HasValue) {
                int duration = (int)TelemetryRefreshInterval.Value.Value;
                telemetryRefreshEndTime = DateTime.Now.AddSeconds(duration);
                telemetryRefreshTotalDuration = duration;

                if (telemetryRefreshTimer == null) {
                    telemetryRefreshTimer = new DispatcherTimer();
                    telemetryRefreshTimer.Interval = TimeSpan.FromSeconds(1);
                    telemetryRefreshTimer.Tick += TelemetryRefreshTimer_Tick;
                }

                telemetryRefreshTimer.Start();

                try {
                    CommandDefinition command = CommandRegistry.GetCommandByName("GetLastRecord");
                    string response = await _commandManager.ExecuteCommandAsync(command);
                    ProcessTelemetryRecord(response);
                }
                catch (Exception ex) {
                    Debug.WriteLine($"Error in initial telemetry fetch: {ex.Message}");
                }

                TelemetryRefreshProgressBar.Maximum = duration;
                TelemetryRefreshProgressBar.Value = duration;
                TelemetryRefreshStatusLabel.Content = $"Pozostało: {duration}s";
                StartTelemetryAutoRefreshButton.Content = "Zatrzymaj";
                isAutoRefreshActive = true;
                TelemetryRefreshInterval.IsEnabled = false;
            }
        }

        private void StopTelemetryAutoRefresh() {
            if (telemetryRefreshTimer != null) {
                telemetryRefreshTimer.Stop();
            }

            TelemetryRefreshProgressBar.Value = 0;
            TelemetryRefreshStatusLabel.Content = "Automat zatrzymany";

            StartTelemetryAutoRefreshButton.Content = "Rozpocznij auto-odświeżanie";
            isAutoRefreshActive = false;

            TelemetryRefreshInterval.IsEnabled = true;
        }

        private async void GetRecentTelemetryButton_Click(object sender, RoutedEventArgs e) {
            try {
                int count = RecentTelemetryCount.Value.HasValue ? (int)RecentTelemetryCount.Value.Value : 5;
                CommandDefinition command = CommandRegistry.GetCommandByName("GetRecentTelemetry");
                string response = await _commandManager.ExecuteCommandAsync(command, count.ToString());
                if (string.IsNullOrEmpty(response)) {
                    DialogHelper.ShowMessage(this, "Brak danych telemetrycznych w buforze.");
                    return;
                }

                string[] records = response.Split('|', StringSplitOptions.RemoveEmptyEntries);
                foreach (string csv in records) {
                    try {
                        ProcessTelemetryRecord(csv);
                    }
                    catch (Exception ex) {
                        Debug.WriteLine($"Skip bad recent telem row: {ex.Message}");
                    }
                }

                DialogHelper.ShowMessage(this, $"Załadowano {records.Length} rekordów z bufora RAM.");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania ostatnich rekordów: {ex.Message}");
            }
        }

        private async void TelemetryRefreshTimer_Tick(object sender, EventArgs e) {
            double remainingTime = (telemetryRefreshEndTime - DateTime.Now).TotalSeconds;

            if (remainingTime <= 0) {
                StopTelemetryAutoRefresh();
                return;
            }

            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetLastRecord");
                string response = await _commandManager.ExecuteCommandAsync(command);
                ProcessTelemetryRecord(response);
            }
            catch (Exception ex) {
                Debug.WriteLine($"Error in telemetry refresh: {ex.Message}");
            }

            TelemetryRefreshProgressBar.Value = remainingTime;
            TelemetryRefreshStatusLabel.Content = $"Pozostało: {remainingTime:F0}s";
        }

        private void DisableOtherTabs() {
            for (int i = 0; i < MainTabControl.Items.Count; i++) {
                TabItem? item = MainTabControl.Items[i] as TabItem;
                if (item != null) {
                    if (item.Header.ToString() != "Telemetria" && item.Header.ToString() != "Log komunikacji") {
                        item.IsEnabled = false;
                    }
                }
            }
        }

        private void EnableAllTabs() {
            for (int i = 0; i < MainTabControl.Items.Count; i++) {
                TabItem? item = MainTabControl.Items[i] as TabItem;
                if (item != null) {
                    item.IsEnabled = true;
                }
            }
        }

        private void ProcessTelemetryRecord(string value) {
            try {
                TelemetryRecord record = TelemetryRecord.FromCsv(value);
                UpdateTelemetryTable(record);
                _telemetryPlotManager.AddDataPoint(record);
                UpdateGpsVisualization(record);
            }
            catch (Exception ex) {
                Debug.WriteLine($"Błąd przetwarzania danych telemetrycznych: {ex.Message}");
                DialogHelper.ShowMessage(this, $"Błąd przetwarzania danych telemetrycznych: {ex.Message}");
            }
        }


        private void UpdateTelemetryTable(TelemetryRecord record) {
            Dispatcher.UIThread.Post(() => {
                TelemetryData.Clear();

                // Timestamp
                DateTime timestamp = DateTimeOffset.FromUnixTimeSeconds(record.Timestamp).DateTime;
                TelemetryData.Add(new KeyValuePair<string, string>("Czas rekordu", timestamp.ToString("yyyy-MM-dd HH:mm:ss")));
                TelemetryData.Add(new KeyValuePair<string, string>("Wersja programu", record.BuildVersion.ToString()));

                // Power data
                TelemetryData.Add(new KeyValuePair<string, string>("Napięcie baterii [V]", record.BatteryVoltage.ToString("F2", CultureInfo.InvariantCulture)));
                TelemetryData.Add(new KeyValuePair<string, string>("Napięcie systemowe [V]", record.SystemVoltage.ToString("F2", CultureInfo.InvariantCulture)));
                TelemetryData.Add(new KeyValuePair<string, string>("Prąd ładowania USB [mA]", record.ChargeCurrentUsb.ToString("F2", CultureInfo.InvariantCulture)));
                TelemetryData.Add(new KeyValuePair<string, string>("Prąd ładowania solarnego [mA]", record.ChargeCurrentSolar.ToString("F2", CultureInfo.InvariantCulture)));
                TelemetryData.Add(new KeyValuePair<string, string>("Prąd rozładowania [mA]", record.DischargeCurrent.ToString("F2", CultureInfo.InvariantCulture)));

                // GPS data — convert NMEA ddmm.mmmmm to decimal degrees
                string fullLat = TryParseNmeaCoordinate(record.Latitude, record.LatDir, out double latDeg)
                    ? $"{Math.Abs(latDeg).ToString("F8", CultureInfo.InvariantCulture)} {(latDeg >= 0 ? "N" : "S")}"
                    : $"{record.Latitude} {record.LatDir}";
                string fullLon = TryParseNmeaCoordinate(record.Longitude, record.LonDir, out double lonDeg)
                    ? $"{Math.Abs(lonDeg).ToString("F8", CultureInfo.InvariantCulture)} {(lonDeg >= 0 ? "E" : "W")}"
                    : $"{record.Longitude} {record.LonDir}";
                TelemetryData.Add(new KeyValuePair<string, string>("Czas GPS", record.Time));
                TelemetryData.Add(new KeyValuePair<string, string>("Szerokość geograficzna", fullLat));
                TelemetryData.Add(new KeyValuePair<string, string>("Długość geograficzna", fullLon));
                TelemetryData.Add(new KeyValuePair<string, string>("Prędkość [m/s]", record.Speed));
                TelemetryData.Add(new KeyValuePair<string, string>("Kurs [°]", record.Course));
                TelemetryData.Add(new KeyValuePair<string, string>("Data GPS", record.Date));
                TelemetryData.Add(new KeyValuePair<string, string>("Jakość sygnału GPS", record.FixQuality));
                TelemetryData.Add(new KeyValuePair<string, string>("Liczba satelitów", record.Satellites));
                TelemetryData.Add(new KeyValuePair<string, string>("Wysokość [m]", record.Altitude));
            });
        }

        private void UpdateGpsVisualization(TelemetryRecord record) {
            string timeString = record.Time;
            string dateString = record.Date;

            if (DateTime.TryParseExact($"{timeString} {dateString}", "HHmmss ddMMyy",
                CultureInfo.InvariantCulture, DateTimeStyles.None, out DateTime gpsDateTime)) {
                GpsTimeDisplay.Text = gpsDateTime.ToString("HH:mm:ss");
                GpsDateDisplay.Text = gpsDateTime.ToString("dd MMMM yyyy");
            }

            GpsSatelliteCount.Text = $"{record.Satellites} satelit";
            UpdateFixQualityDisplay(record.FixQuality);

            UpdatePosition(record);

            UpdatePositionDisplay(record);
        }

        private void UpdateFixQualityDisplay(string fixQuality) {
            string qualityText = fixQuality switch {
                "0" => "Brak fix",
                "1" => "GPS fix",
                "2" => "DGPS fix",
                _ => "Nieznany status"
            };

            GpsFixQuality.Text = qualityText;
            GpsFixQuality.Foreground = fixQuality == "0"
                ? Brushes.Red
                : fixQuality == "1" ? Brushes.Green : Brushes.Blue;
        }

        private void UpdatePosition(TelemetryRecord record) {
            if (TryParseNmeaCoordinate(record.Latitude, record.LatDir, out double latDeg) &&
                TryParseNmeaCoordinate(record.Longitude, record.LonDir, out double lonDeg) &&
                double.TryParse(record.Altitude, NumberStyles.Any, CultureInfo.InvariantCulture, out double alt)) {
                GpsPositionSummary.Text = $"Pozycja: {FormatCoordinate(latDeg, "N", "S")}, {FormatCoordinate(lonDeg, "E", "W")}\nWysokość: {alt:F1}m";
            }
            else {
                Debug.WriteLine("Failed to parse GPS coordinates");
            }
        }


        private bool TryParseNmeaCoordinate(string coord, string direction, out double degrees) {
            degrees = 0;
            if (string.IsNullOrEmpty(coord) || string.IsNullOrEmpty(direction))
                return false;

            int degreeDigits = coord.Length >= 5 ? (direction == "N" || direction == "S" ? 2 : 3) : 0;

            if (degreeDigits == 0 || !double.TryParse(coord, NumberStyles.Any, CultureInfo.InvariantCulture, out double rawValue))
                return false;

            double deg = Math.Floor(rawValue / 100);
            double minutes = rawValue - (deg * 100);

            degrees = deg + (minutes / 60);

            if (direction == "S" || direction == "W")
                degrees = -degrees;

            return true;
        }

        private string FormatCoordinate(double coordinate, string positiveDir, string negativeDir) {
            string direction = coordinate >= 0 ? positiveDir : negativeDir;
            coordinate = Math.Abs(coordinate);

            int degrees = (int)Math.Floor(coordinate);
            double minutes = (coordinate - degrees) * 60;

            return $"{degrees}°{minutes:F3}' {direction}";
        }

        private async void ExportVoltagePlotButton_Click(object sender, RoutedEventArgs e) {
            string? fileName = await DialogHelper.SaveFileAsync(this, "Eksport wykresu", $"voltage_data_{DateTime.Now:yyyyMMdd_HHmmss}.png", DialogHelper.PngFileType);
            if (fileName == null) return;
            try {
                _sensorPlotManager.ExportToPng(_telemetryPlotManager.VoltagePlotModel, fileName);
                DialogHelper.ShowMessage(this, "Wykres został wyeksportowany pomyślnie.");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd podczas eksportu wykresu: {ex.Message}");
            }
        }

        private async void ExportCurrentPlotButton_Click(object sender, RoutedEventArgs e) {
            string? fileName = await DialogHelper.SaveFileAsync(this, "Eksport wykresu", $"current_data_{DateTime.Now:yyyyMMdd_HHmmss}.png", DialogHelper.PngFileType);
            if (fileName == null) return;
            try {
                _sensorPlotManager.ExportToPng(_telemetryPlotManager.CurrentPlotModel, fileName);
                DialogHelper.ShowMessage(this, "Wykres został wyeksportowany pomyślnie.");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd podczas eksportu wykresu: {ex.Message}");
            }
        }

    }
}

