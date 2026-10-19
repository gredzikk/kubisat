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
    public class SensorDataRecord {
        public uint Timestamp { get; set; }
        public float Temperature { get; set; }
        public float Pressure { get; set; }
        public float Humidity { get; set; }
        public float Light { get; set; }
        public float Ax { get; set; }
        public float Ay { get; set; }
        public float Az { get; set; }
        public float Gx { get; set; }
        public float Gy { get; set; }
        public float Gz { get; set; }

        public static SensorDataRecord FromCsv(string csvData) {
            string[] parts = csvData.Split(',');
            if (parts.Length < 5)
                throw new ArgumentException("Nieprawidłowy format danych z czujników");

            var record = new SensorDataRecord {
                Timestamp = uint.Parse(parts[0]),
                Temperature = float.Parse(parts[1], CultureInfo.InvariantCulture),
                Pressure = float.Parse(parts[2], CultureInfo.InvariantCulture),
                Humidity = float.Parse(parts[3], CultureInfo.InvariantCulture),
                Light = float.Parse(parts[4], CultureInfo.InvariantCulture)
            };

            // Firmware sensor CSV: ts,temp,press,hum,lux,ax,ay,az,gx,gy,gz
            if (parts.Length >= 11) {
                record.Ax = float.Parse(parts[5], CultureInfo.InvariantCulture);
                record.Ay = float.Parse(parts[6], CultureInfo.InvariantCulture);
                record.Az = float.Parse(parts[7], CultureInfo.InvariantCulture);
                record.Gx = float.Parse(parts[8], CultureInfo.InvariantCulture);
                record.Gy = float.Parse(parts[9], CultureInfo.InvariantCulture);
                record.Gz = float.Parse(parts[10], CultureInfo.InvariantCulture);
            }

            return record;
        }
    }

    public class SensorPlotManager {
        private PlotModel _temperatureHumidityPlotModel;
        private PlotModel _lightPlotModel;
        private PlotModel _pressurePlotModel;
        private LineSeries _temperatureSeries;
        private LineSeries _pressureSeries;
        private LineSeries _humiditySeries;
        private LineSeries _lightSeries;
        private DateTimeAxis _environmentTimeAxis;
        private DateTimeAxis _lightTimeAxis;
        private DateTimeAxis _pressureTimeAxis;

        public PlotModel EnvironmentPlotModel => _temperatureHumidityPlotModel;
        public PlotModel LightPlotModel => _lightPlotModel;
        public PlotModel PressurePlotModel => _pressurePlotModel;

        public SensorPlotManager() {
            InitializeEnvironmentPlot();
            InitializeLightPlot();
            InitializePressurePlot();
        }

        private void InitializeEnvironmentPlot() {
            _temperatureHumidityPlotModel = new PlotModel { Title = "Temperatura i wilgotność" };

            _temperatureHumidityPlotModel.Legends.Add(new Legend {
                LegendPosition = LegendPosition.RightTop,
                LegendPlacement = LegendPlacement.Outside,
                LegendOrientation = LegendOrientation.Vertical,
                LegendBorder = OxyColors.Transparent
            });

            _environmentTimeAxis = new DateTimeAxis {
                Position = AxisPosition.Bottom,
                Title = "Czas",
                StringFormat = "HH:mm:ss",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            LinearAxis valueAxis = new LinearAxis {
                Position = AxisPosition.Left,
                Title = "Wartość",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            _temperatureHumidityPlotModel.Axes.Add(_environmentTimeAxis);
            _temperatureHumidityPlotModel.Axes.Add(valueAxis);

            _temperatureSeries = new LineSeries {
                Title = "Temperatura [°C]",
                Color = OxyColor.FromRgb(255, 0, 0), // Red
                StrokeThickness = 2
            };



            _humiditySeries = new LineSeries {
                Title = "Wilgotność [%]",
                Color = OxyColor.FromRgb(0, 255, 0), // Green
                StrokeThickness = 2
            };

            _temperatureHumidityPlotModel.Series.Add(_temperatureSeries);
            _temperatureHumidityPlotModel.Series.Add(_humiditySeries);
        }

        private void InitializePressurePlot() {
            _pressurePlotModel = new PlotModel { Title = "Ciśnienie" };

            _pressurePlotModel.Legends.Add(new Legend {
                LegendPosition = LegendPosition.RightTop,
                LegendPlacement = LegendPlacement.Outside,
                LegendOrientation = LegendOrientation.Vertical,
                LegendBorder = OxyColors.Transparent
            });

            _pressureTimeAxis = new DateTimeAxis {
                Position = AxisPosition.Bottom,
                Title = "Czas",
                StringFormat = "HH:mm:ss",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            LinearAxis pressureValueAxis = new LinearAxis {
                Position = AxisPosition.Left,
                Title = "Ciśnienie [hPa]",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            _pressurePlotModel.Axes.Add(_pressureTimeAxis);
            _pressurePlotModel.Axes.Add(pressureValueAxis);

            _pressureSeries = new LineSeries {
                Title = "Ciśnienie [hPa]",
                Color = OxyColor.FromRgb(0, 0, 255), // Blue
                StrokeThickness = 2
            };

            _pressurePlotModel.Series.Add(_pressureSeries);
        }
        private void InitializeLightPlot() {
            _lightPlotModel = new PlotModel { Title = "Natężenie światła" };

            _lightPlotModel.Legends.Add(new Legend {
                LegendPosition = LegendPosition.RightTop,
                LegendPlacement = LegendPlacement.Outside,
                LegendOrientation = LegendOrientation.Vertical,
                LegendBorder = OxyColors.Transparent
            });

            _lightTimeAxis = new DateTimeAxis {
                Position = AxisPosition.Bottom,
                Title = "Czas",
                StringFormat = "HH:mm:ss",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            LinearAxis lightValueAxis = new LinearAxis {
                Position = AxisPosition.Left,
                Title = "Natężenie światła [lux]",
                MajorGridlineStyle = LineStyle.Solid,
                MinorGridlineStyle = LineStyle.Dot,
                MajorGridlineColor = OxyColor.FromRgb(200, 200, 200),
                MinorGridlineColor = OxyColor.FromRgb(230, 230, 230)
            };

            _lightPlotModel.Axes.Add(_lightTimeAxis);
            _lightPlotModel.Axes.Add(lightValueAxis);

            _lightSeries = new LineSeries {
                Title = "Natężenie światła [lux]",
                Color = OxyColor.FromRgb(255, 215, 0), // Gold
                StrokeThickness = 2
            };

            _lightPlotModel.Series.Add(_lightSeries);
        }

        public void AddDataPoint(SensorDataRecord record) {
            DateTime time = DateTimeOffset.FromUnixTimeSeconds(record.Timestamp).DateTime;
            double timeValue = DateTimeAxis.ToDouble(time);

            _temperatureSeries.Points.Add(new DataPoint(timeValue, record.Temperature));
            _pressureSeries.Points.Add(new DataPoint(timeValue, record.Pressure));
            _humiditySeries.Points.Add(new DataPoint(timeValue, record.Humidity));
            _lightSeries.Points.Add(new DataPoint(timeValue, record.Light));

            _temperatureHumidityPlotModel.InvalidatePlot(true);
            _lightPlotModel.InvalidatePlot(true);
            _pressurePlotModel.InvalidatePlot(true);
        }

        public void ClearData() {
            _temperatureSeries.Points.Clear();
            _pressureSeries.Points.Clear();
            _humiditySeries.Points.Clear();
            _lightSeries.Points.Clear();
            _pressureSeries.Points.Clear();

            _temperatureHumidityPlotModel.InvalidatePlot(true);
            _lightPlotModel.InvalidatePlot(true);
            _pressurePlotModel.InvalidatePlot(true);
        }

        public void ExportToPng(PlotModel plotModel, string filePath) {
            var pngExporter = new OxyPlot.SkiaSharp.PngExporter { Width = 1920, Height = 1080 };
            using var stream = File.Create(filePath);
            pngExporter.Export(plotModel, stream);
        }

        public void LoadFromCsv(string filePath) {
            ClearData();
            string[] lines = File.ReadAllLines(filePath);
            foreach (string line in lines) {
                try {
                    SensorDataRecord record = SensorDataRecord.FromCsv(line);
                    AddDataPoint(record);
                }
                catch (Exception ex) {
                    Debug.WriteLine($"Error parsing line: {line}, Error: {ex.Message}");
                }
            }
        }
    }

    public partial class MainWindow : Window {
        public ObservableCollection<KeyValuePair<string, string>> SensorData { get; set; } =
            new ObservableCollection<KeyValuePair<string, string>>();
        private SensorPlotManager _sensorPlotManager;

        private DispatcherTimer sensorsRefreshTimer;
        private DateTime sensorsRefreshEndTime;
        private double sensorsRefreshTotalDuration;
        private bool isSensorsAutoRefreshActive = false;

        private async void GetLastSensorDataButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetLastSensorData");
                string response = await _commandManager.ExecuteCommandAsync(command);

                ProcessSensorDataResponse(response);
            }
            catch (Exception ex) {
                Debug.WriteLine($"Error getting sensor data: {ex.Message}");
                DialogHelper.ShowMessage(this, $"Błąd pobierania danych z czujników: {ex.Message}");
            }
        }

        private void StartSensorAutoRefreshButton_Click(object sender, RoutedEventArgs e) {
            if (isSensorsAutoRefreshActive) {
                StopSensorAutoRefresh();
            }
            else {
                StartSensorAutoRefresh();
            }
        }

        private async void StartSensorAutoRefresh() {
            if (SensorRefreshInterval.Value.HasValue) {
                int duration = (int)SensorRefreshInterval.Value.Value;
                sensorsRefreshEndTime = DateTime.Now.AddSeconds(duration);
                sensorsRefreshTotalDuration = duration;

                if (sensorsRefreshTimer == null) {
                    sensorsRefreshTimer = new DispatcherTimer();
                    sensorsRefreshTimer.Interval = TimeSpan.FromSeconds(1);
                    sensorsRefreshTimer.Tick += SensorsRefreshTimer_Tick;
                }

                sensorsRefreshTimer.Start();

                try {
                    CommandDefinition command = CommandRegistry.GetCommandByName("GetLastSensorData");
                    string response = await _commandManager.ExecuteCommandAsync(command);
                    ProcessSensorDataResponse(response);
                }
                catch (Exception ex) {
                    Debug.WriteLine($"Error in initial sensor fetch: {ex.Message}");
                }

                SensorRefreshProgressBar.Maximum = duration;
                SensorRefreshProgressBar.Value = duration;
                SensorRefreshStatusLabel.Content = $"Pozostało: {duration}s";
                StartSensorAutoRefreshButton.Content = "Zatrzymaj";
                isSensorsAutoRefreshActive = true;
                SensorRefreshInterval.IsEnabled = false;
            }
            else {
                DialogHelper.ShowMessage(this, "Wprowadź prawidłową wartość interwału odświeżania.");
            }
        }

        private void StopSensorAutoRefresh() {
            if (sensorsRefreshTimer != null) {
                sensorsRefreshTimer.Stop();
            }

            SensorRefreshProgressBar.Value = 0;
            SensorRefreshStatusLabel.Content = "Automat zatrzymany";

            StartSensorAutoRefreshButton.Content = "Rozpocznij auto-odświeżanie";
            isSensorsAutoRefreshActive = false;

            SensorRefreshInterval.IsEnabled = true;
        }

        private async void SensorsRefreshTimer_Tick(object sender, EventArgs e) {
            double remainingTime = (sensorsRefreshEndTime - DateTime.Now).TotalSeconds;

            if (remainingTime <= 0) {
                StopSensorAutoRefresh();
                return;
            }

            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetLastSensorData");
                string response = await _commandManager.ExecuteCommandAsync(command);
                ProcessSensorDataResponse(response);
            }
            catch (Exception ex) {
                Debug.WriteLine($"Error in sensor refresh: {ex.Message}");
            }

            SensorRefreshProgressBar.Value = remainingTime;
            SensorRefreshStatusLabel.Content = $"Pozostało: {remainingTime:F0}s";
        }

        private void DisableTabs() {
            for (int i = 0; i < MainTabControl.Items.Count; i++) {
                TabItem? item = MainTabControl.Items[i] as TabItem;
                if (item != null) {
                    if (item.Header.ToString() != "Sensory" && item.Header.ToString() != "Log komunikacji") {
                        item.IsEnabled = false;
                    }
                }
            }
        }

        private void ProcessSensorDataResponse(string value) {
            try {
                SensorDataRecord record = SensorDataRecord.FromCsv(value);
                UpdateSensorTable(record);
                _sensorPlotManager.AddDataPoint(record);
            }
            catch (Exception ex) {
                Debug.WriteLine($"Błąd przetwarzania danych z czujników: {ex.Message}");
                DialogHelper.ShowMessage(this, $"Błąd przetwarzania danych z czujników: {ex.Message}");
            }
        }

        private void UpdateSensorTable(SensorDataRecord record) {
            SensorData.Clear();

            DateTime timestamp = DateTimeOffset.FromUnixTimeSeconds(record.Timestamp).DateTime;
            SensorData.Add(new KeyValuePair<string, string>("Czas rekordu", timestamp.ToString("yyyy-MM-dd HH:mm:ss")));

            SensorData.Add(new KeyValuePair<string, string>("Temperatura [°C]", record.Temperature.ToString("F1", CultureInfo.InvariantCulture)));
            SensorData.Add(new KeyValuePair<string, string>("Ciśnienie [hPa]", record.Pressure.ToString("F1", CultureInfo.InvariantCulture)));
            SensorData.Add(new KeyValuePair<string, string>("Wilgotność [%]", record.Humidity.ToString("F1", CultureInfo.InvariantCulture)));
            SensorData.Add(new KeyValuePair<string, string>("Natężenie światła [lux]", record.Light.ToString("F1", CultureInfo.InvariantCulture)));
            SensorData.Add(new KeyValuePair<string, string>("Accel ax,ay,az [g]",
                $"{record.Ax:F3},{record.Ay:F3},{record.Az:F3}"));
            SensorData.Add(new KeyValuePair<string, string>("Gyro gx,gy,gz [dps]",
                $"{record.Gx:F3},{record.Gy:F3},{record.Gz:F3}"));
        }

        

        private async void ExportEnvironmentPlotButton_Click(object sender, RoutedEventArgs e) {
            string? fileName = await DialogHelper.SaveFileAsync(this, "Eksport wykresu", $"environmental_data_{DateTime.Now:yyyyMMdd_HHmmss}.png", DialogHelper.PngFileType);
            if (fileName == null) return;
            try {
                _sensorPlotManager.ExportToPng(_sensorPlotManager.EnvironmentPlotModel, fileName);
                DialogHelper.ShowMessage(this, "Wykres został wyeksportowany pomyślnie.");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd podczas eksportu wykresu: {ex.Message}");
            }
        }

        private async void ExportLightPlotButton_Click(object sender, RoutedEventArgs e) {
            string? fileName = await DialogHelper.SaveFileAsync(this, "Eksport wykresu", $"light_data_{DateTime.Now:yyyyMMdd_HHmmss}.png", DialogHelper.PngFileType);
            if (fileName == null) return;
            try {
                _sensorPlotManager.ExportToPng(_sensorPlotManager.LightPlotModel, fileName);
                DialogHelper.ShowMessage(this, "Wykres został wyeksportowany pomyślnie.");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd podczas eksportu wykresu: {ex.Message}");
            }
        }

        private async void ExportPressurePlotButton_Click(object sender, RoutedEventArgs e) {
            string? fileName = await DialogHelper.SaveFileAsync(this, "Eksport wykresu", $"pressure_data_{DateTime.Now:yyyyMMdd_HHmmss}.png", DialogHelper.PngFileType);
            if (fileName == null) return;
            try {
                _sensorPlotManager.ExportToPng(_sensorPlotManager.PressurePlotModel, fileName);
                DialogHelper.ShowMessage(this, "Wykres został wyeksportowany pomyślnie.");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd podczas eksportu wykresu: {ex.Message}");
            }
        }
    }
}
