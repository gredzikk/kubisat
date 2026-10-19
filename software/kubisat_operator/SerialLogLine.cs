using Avalonia.Media;

namespace kubisat_operator;

public class SerialLogLine {
    public string Text { get; set; } = string.Empty;
    public IBrush Foreground { get; set; } = Brushes.Black;
}
