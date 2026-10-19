using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Platform.Storage;

namespace kubisat_operator;

public static class DialogHelper {
    public static async Task ShowMessageAsync(Window? owner, string message, string title = "Informacja") {
        Window dialog = new Window {
            Title = title,
            Width = 420,
            Height = 180,
            WindowStartupLocation = WindowStartupLocation.CenterOwner,
            CanResize = false,
            Content = BuildMessageContent(message, out Button okButton)
        };

        okButton.Click += (_, _) => dialog.Close();

        if (owner != null)
            await dialog.ShowDialog(owner);
        else
            dialog.Show();
    }

    public static void ShowMessage(Window? owner, string message, string title = "Informacja") {
        _ = ShowMessageAsync(owner, message, title);
    }

    public static async Task<bool> ShowConfirmAsync(Window? owner, string message, string title = "Potwierdzenie") {
        bool result = false;
        Window dialog = new Window {
            Title = title,
            Width = 420,
            Height = 180,
            WindowStartupLocation = WindowStartupLocation.CenterOwner,
            CanResize = false
        };

        Button okButton = new Button { Content = "OK", Width = 80, Margin = new Thickness(0, 0, 10, 0) };
        Button cancelButton = new Button { Content = "Anuluj", Width = 80 };
        okButton.Click += (_, _) => { result = true; dialog.Close(); };
        cancelButton.Click += (_, _) => dialog.Close();

        dialog.Content = new StackPanel {
            Margin = new Thickness(16),
            Spacing = 16,
            Children = {
                new TextBlock { Text = message, TextWrapping = TextWrapping.Wrap },
                new StackPanel {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Right,
                    Children = { okButton, cancelButton }
                }
            }
        };

        if (owner != null)
            await dialog.ShowDialog(owner);
        else
            dialog.Show();

        return result;
    }

    public static async Task<string?> OpenFileAsync(Window owner, string title, params FilePickerFileType[] filters) {
        IReadOnlyList<IStorageFile> files = await owner.StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions {
            Title = title,
            AllowMultiple = false,
            FileTypeFilter = filters.Length > 0 ? filters : null
        });

        return files.Count > 0 ? files[0].TryGetLocalPath() : null;
    }

    public static async Task<string?> SaveFileAsync(Window owner, string title, string? suggestedFileName = null, params FilePickerFileType[] filters) {
        IStorageFile? file = await owner.StorageProvider.SaveFilePickerAsync(new FilePickerSaveOptions {
            Title = title,
            SuggestedFileName = suggestedFileName,
            FileTypeChoices = filters.Length > 0 ? filters : null
        });

        return file?.TryGetLocalPath();
    }

    public static FilePickerFileType CsvFileType { get; } = new("CSV") {
        Patterns = new[] { "*.csv" }
    };

    public static FilePickerFileType PngFileType { get; } = new("PNG") {
        Patterns = new[] { "*.png" }
    };

    public static FilePickerFileType KmlFileType { get; } = new("KML") {
        Patterns = new[] { "*.kml" }
    };

    public static FilePickerFileType TextFileType { get; } = new("Tekst") {
        Patterns = new[] { "*.txt", "*.log" }
    };

    private static Control BuildMessageContent(string message, out Button okButton) {
        okButton = new Button {
            Content = "OK",
            Width = 80,
            HorizontalAlignment = HorizontalAlignment.Right
        };

        return new StackPanel {
            Margin = new Thickness(16),
            Spacing = 16,
            Children = {
                new TextBlock { Text = message, TextWrapping = TextWrapping.Wrap },
                okButton
            }
        };
    }
}
