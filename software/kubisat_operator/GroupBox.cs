using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;

namespace kubisat_operator;

/// <summary>
/// Simple GroupBox stand-in for WPF GroupBox (header + bordered content).
/// </summary>
public class GroupBox : HeaderedContentControl {
    protected override Type StyleKeyOverride => typeof(GroupBox);
}
