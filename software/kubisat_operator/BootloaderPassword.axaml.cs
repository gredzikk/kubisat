using Avalonia.Controls;
using Avalonia.Interactivity;

namespace kubisat_operator;

public partial class PasswordDialog : Window {
    public string Password { get; private set; } = string.Empty;
    public bool Confirmed { get; private set; }

    public PasswordDialog() {
        InitializeComponent();
    }

    private void OkButton_Click(object? sender, RoutedEventArgs e) {
        Password = PasswordBox.Text ?? string.Empty;
        Confirmed = true;
        Close(true);
    }

    private void CancelButton_Click(object? sender, RoutedEventArgs e) {
        Confirmed = false;
        Close(false);
    }
}
