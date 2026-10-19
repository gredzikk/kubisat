using System.Diagnostics;
using System.IO;
using System.Text;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;

namespace kubisat_operator;

public partial class MainWindow : Window {
    private bool _autoScroll = true;
    private bool _showTimestamps = true;

    private void FontSizeSelector_SelectionChanged(object? sender, SelectionChangedEventArgs e) {
        if (SerialMonitorItems != null && FontSizeSelector.SelectedItem is ComboBoxItem selectedItem) {
            if (int.TryParse(selectedItem.Content?.ToString(), out int fontSize)) {
                SerialMonitorItems.FontSize = fontSize;
            }
        }
    }

    private void ClearSerialMonitor_Click(object? sender, RoutedEventArgs e) {
        _serialLogLines.Clear();
    }

    private async void SaveSerialMonitor_Click(object? sender, RoutedEventArgs e) {
        string? fileName = await DialogHelper.SaveFileAsync(this, "Zapisz log komunikacji", "serial_log.txt", DialogHelper.TextFileType);
        if (fileName == null) return;

        try {
            StringBuilder sb = new StringBuilder();
            foreach (SerialLogLine line in _serialLogLines) {
                sb.AppendLine(line.Text);
            }

            await File.WriteAllTextAsync(fileName, sb.ToString());
            DialogHelper.ShowMessage(this, $"Log zapisany do pliku: {fileName}", "Zapisano");
        }
        catch (Exception ex) {
            DialogHelper.ShowMessage(this, $"Błąd podczas zapisywania pliku: {ex.Message}", "Błąd");
        }
    }

    private void ShowTimestampsCheckBox_Changed(object? sender, RoutedEventArgs e) {
        _showTimestamps = ShowTimestampsCheckBox.IsChecked ?? true;
    }

    private StringBuilder _serialBuffer = new StringBuilder();

    private static IBrush ResolveBrush(string colorName) {
        return colorName switch {
            "Red" => Brushes.Red,
            "Blue" => Brushes.Blue,
            "Green" => Brushes.Green,
            "Orange" => Brushes.Orange,
            "Black" => Brushes.Black,
            _ => Brushes.Black
        };
    }

    private void AppendColoredText(string text, string colorName) {
        void Append() {
            try {
                string[] lines = text.Replace("\r\n", "\n").Replace('\r', '\n').Split('\n');
                IBrush brush = ResolveBrush(colorName);

                foreach (string line in lines) {
                    if (string.IsNullOrEmpty(line) && lines.Length == 1)
                        continue;
                    _serialLogLines.Add(new SerialLogLine { Text = line, Foreground = brush });
                }

                if (AutoScrollCheckBox.IsChecked == true && _serialLogLines.Count > 0) {
                    SerialMonitorScrollViewer.ScrollToEnd();
                }
            }
            catch (Exception ex) {
                Debug.WriteLine($"Error appending colored text: {ex.Message}");
            }
        }

        if (Dispatcher.UIThread.CheckAccess())
            Append();
        else
            Dispatcher.UIThread.Post(Append);
    }
}
