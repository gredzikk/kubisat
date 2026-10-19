using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Text;
using Avalonia.Controls;
using Avalonia.Interactivity;

namespace kubisat_operator;

public class GpsPosition {
    public DateTime Timestamp { get; set; }
    public double Latitude { get; set; }
    public double Longitude { get; set; }
    public double Altitude { get; set; }
    public string Satellites { get; set; } = string.Empty;
    public float BatteryVoltage { get; set; }
    public float SystemVoltage { get; set; }
    public float ChargeCurrentUsb { get; set; }
    public float ChargeCurrentSolar { get; set; }
    public float DischargeCurrent { get; set; }

    public override string ToString() {
        return $"{Timestamp:yyyy-MM-dd HH:mm:ss}, {Latitude:F6}, {Longitude:F6}, {Altitude:F1} m, {Satellites} sat.";
    }
}

public partial class MainWindow : Window {
    private bool _isTrackingEnabled = false;
    private GpsPosition? _currentPosition = null;
    private ObservableCollection<GpsPosition> _positionHistory = new ObservableCollection<GpsPosition>();

    private void InitializeGpsVisualization() {
        PositionHistoryGrid.ItemsSource = _positionHistory;
        ViewInMapsButton.IsEnabled = false;
    }

    private void TrackSatellite_Click(object? sender, RoutedEventArgs e) {
        _isTrackingEnabled = !_isTrackingEnabled;
        TrackSatelliteButton.Content = _isTrackingEnabled ? "Zatrzymaj śledzenie" : "Śledzenie satelity";

        if (_currentPosition != null) {
            AddPositionToHistory(_currentPosition);
        }
    }

    private void ClearTracking_Click(object? sender, RoutedEventArgs e) {
        _positionHistory.Clear();
        GpsPositionSummary.Text = _currentPosition != null
            ? $"Pozycja: {FormatCoordinate(_currentPosition.Latitude, "N", "S")}, {FormatCoordinate(_currentPosition.Longitude, "E", "W")}\nWysokość: {_currentPosition.Altitude:F1}m"
            : "Brak danych GPS";
    }

    private async void ExportKml_Click(object? sender, RoutedEventArgs e) {
        if (_positionHistory.Count == 0) {
            DialogHelper.ShowMessage(this, "Brak danych pozycji do eksportu.", "Eksport KML");
            return;
        }

        string? fileName = await DialogHelper.SaveFileAsync(
            this,
            "Eksport KML",
            $"kubisat_trajectory_{DateTime.Now:yyyyMMdd_HHmmss}.kml",
            DialogHelper.KmlFileType);

        if (fileName == null) return;

        try {
            ExportToKml(fileName);
            DialogHelper.ShowMessage(this, "Plik KML został pomyślnie wyeksportowany.", "Eksport KML");
        }
        catch (Exception ex) {
            DialogHelper.ShowMessage(this, $"Błąd podczas eksportu KML: {ex.Message}", "Błąd");
        }
    }

    private async void ViewInMaps_Click(object? sender, RoutedEventArgs e) {
        if (_currentPosition == null) {
            DialogHelper.ShowMessage(this, "Brak aktualnych danych GPS.", "Informacja");
            return;
        }

        string googleMapsUrl = $"https://www.google.com/maps?q={_currentPosition.Latitude.ToString(CultureInfo.InvariantCulture)},{_currentPosition.Longitude.ToString(CultureInfo.InvariantCulture)}";

        try {
            Process.Start(new ProcessStartInfo {
                FileName = googleMapsUrl,
                UseShellExecute = true
            });
        }
        catch (Exception ex) {
            DialogHelper.ShowMessage(this, $"Nie można otworzyć przeglądarki: {ex.Message}", "Błąd");

            var clipboard = TopLevel.GetTopLevel(this)?.Clipboard;
            if (clipboard != null)
                await clipboard.SetTextAsync(googleMapsUrl);

            DialogHelper.ShowMessage(this, $"URL został skopiowany do schowka: {googleMapsUrl}", "Informacja");
        }
    }

    private void UpdatePositionDisplay(TelemetryRecord record) {
        if (TryParseNmeaCoordinate(record.Latitude, record.LatDir, out double latitude) &&
            TryParseNmeaCoordinate(record.Longitude, record.LonDir, out double longitude) &&
            double.TryParse(record.Altitude, NumberStyles.Any, CultureInfo.InvariantCulture, out double altitude)) {
            DateTime timestamp = DateTime.Now;
            if (DateTime.TryParseExact($"{record.Time} {record.Date}", "HHmmss ddMMyy",
                CultureInfo.InvariantCulture, DateTimeStyles.None, out DateTime gpsDateTime)) {
                timestamp = gpsDateTime;
            }

            _currentPosition = new GpsPosition {
                Timestamp = timestamp,
                Latitude = latitude,
                Longitude = longitude,
                Altitude = altitude,
                Satellites = record.Satellites,
                BatteryVoltage = record.BatteryVoltage,
                SystemVoltage = record.SystemVoltage,
                ChargeCurrentUsb = record.ChargeCurrentUsb,
                ChargeCurrentSolar = record.ChargeCurrentSolar,
                DischargeCurrent = record.DischargeCurrent
            };

            GpsPositionSummary.Text = $"Pozycja: {FormatCoordinate(latitude, "N", "S")}, {FormatCoordinate(longitude, "E", "W")}\nWysokość: {altitude:F1}m\nSatelity: {record.Satellites}";
            ViewInMapsButton.IsEnabled = true;
            if (_isTrackingEnabled)
                AddPositionToHistory(_currentPosition);
        }
        else {
            GpsPositionSummary.Text = "Nieprawidłowe dane GPS";
            ViewInMapsButton.IsEnabled = false;
        }
    }

    private void AddPositionToHistory(GpsPosition position) {
        _positionHistory.Insert(0, position);
    }

    private void ExportToKml(string filename) {
        using (StreamWriter writer = new StreamWriter(filename, false, Encoding.UTF8)) {
            writer.WriteLine("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            writer.WriteLine("<kml xmlns=\"http://www.opengis.net/kml/2.2\">");
            writer.WriteLine("<Document>");
            writer.WriteLine("  <name>KubiSat GPS Track</name>");
            writer.WriteLine("  <description>Track exported from KubiSat Operator</description>");

            writer.WriteLine("  <Style id=\"yellowLineGreenPoly\">");
            writer.WriteLine("    <LineStyle>");
            writer.WriteLine("      <color>7f00ffff</color>");
            writer.WriteLine("      <width>4</width>");
            writer.WriteLine("    </LineStyle>");
            writer.WriteLine("    <PolyStyle>");
            writer.WriteLine("      <color>7f00ff00</color>");
            writer.WriteLine("    </PolyStyle>");
            writer.WriteLine("  </Style>");

            if (_positionHistory.Count > 1) {
                writer.WriteLine("  <Placemark>");
                writer.WriteLine("    <name>KubiSat Trajectory</name>");
                writer.WriteLine("    <description>GPS trajectory recorded from KubiSat</description>");
                writer.WriteLine("    <styleUrl>#yellowLineGreenPoly</styleUrl>");
                writer.WriteLine("    <LineString>");
                writer.WriteLine("      <extrude>1</extrude>");
                writer.WriteLine("      <tessellate>1</tessellate>");
                writer.WriteLine("      <altitudeMode>absolute</altitudeMode>");
                writer.WriteLine("      <coordinates>");

                for (int i = _positionHistory.Count - 1; i >= 0; i--) {
                    GpsPosition pos = _positionHistory[i];
                    writer.WriteLine($"        {pos.Longitude.ToString("F6", CultureInfo.InvariantCulture)},{pos.Latitude.ToString("F6", CultureInfo.InvariantCulture)},{pos.Altitude.ToString("F1", CultureInfo.InvariantCulture)}");
                }

                writer.WriteLine("      </coordinates>");
                writer.WriteLine("    </LineString>");
                writer.WriteLine("  </Placemark>");
            }

            foreach (GpsPosition pos in _positionHistory) {
                writer.WriteLine("  <Placemark>");
                writer.WriteLine($"    <name>{pos.Timestamp:yyyy-MM-dd HH:mm:ss}</name>");
                writer.WriteLine("    <ExtendedData>");
                writer.WriteLine($"      <Data name=\"Timestamp\">");
                writer.WriteLine($"        <value>{pos.Timestamp:yyyy-MM-dd HH:mm:ss}</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"Latitude\">");
                writer.WriteLine($"        <value>{pos.Latitude.ToString("F6", CultureInfo.InvariantCulture)}</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"Longitude\">");
                writer.WriteLine($"        <value>{pos.Longitude.ToString("F6", CultureInfo.InvariantCulture)}</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"Altitude\">");
                writer.WriteLine($"        <value>{pos.Altitude.ToString("F1", CultureInfo.InvariantCulture)} m</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"Satellites\">");
                writer.WriteLine($"        <value>{pos.Satellites}</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"BatteryVoltage\">");
                writer.WriteLine($"        <value>{pos.BatteryVoltage.ToString("F2", CultureInfo.InvariantCulture)} V</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"SystemVoltage\">");
                writer.WriteLine($"        <value>{pos.SystemVoltage.ToString("F2", CultureInfo.InvariantCulture)} V</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"ChargeCurrentUsb\">");
                writer.WriteLine($"        <value>{pos.ChargeCurrentUsb.ToString("F2", CultureInfo.InvariantCulture)} mA</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"ChargeCurrentSolar\">");
                writer.WriteLine($"        <value>{pos.ChargeCurrentSolar.ToString("F2", CultureInfo.InvariantCulture)} mA</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine($"      <Data name=\"DischargeCurrent\">");
                writer.WriteLine($"        <value>{pos.DischargeCurrent.ToString("F2", CultureInfo.InvariantCulture)} mA</value>");
                writer.WriteLine("      </Data>");
                writer.WriteLine("    </ExtendedData>");
                writer.WriteLine("    <Point>");
                writer.WriteLine("      <altitudeMode>absolute</altitudeMode>");
                writer.WriteLine($"      <coordinates>{pos.Longitude.ToString("F6", CultureInfo.InvariantCulture)},{pos.Latitude.ToString("F6", CultureInfo.InvariantCulture)},{pos.Altitude.ToString("F1", CultureInfo.InvariantCulture)}</coordinates>");
                writer.WriteLine("    </Point>");
                writer.WriteLine("  </Placemark>");
            }

            writer.WriteLine("</Document>");
            writer.WriteLine("</kml>");
        }
    }
}
