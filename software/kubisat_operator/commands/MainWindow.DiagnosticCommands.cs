using System.Collections.ObjectModel;
using System.Diagnostics;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;

namespace kubisat_operator {
    public partial class MainWindow : Window {
        public ObservableCollection<CommandViewModel> DiagnosticCommands { get; set; }

        #region diagnostic commands
        private async void GetCommandsListButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetCommandsList");
                string response = await _commandManager.ExecuteCommandAsync(command);

                ObservableCollection<CommandViewModel> commands = new ObservableCollection<CommandViewModel>();
                response = response.Replace(";", "-");
                Debug.WriteLine(response);

                string[] commandEntries = response.Split('-', StringSplitOptions.RemoveEmptyEntries);

                foreach (string commandString in commandEntries) {
                    string[] groupAndCommand = commandString.Split('.');
                    if (groupAndCommand.Length == 2) {
                        if (byte.TryParse(groupAndCommand[0], out byte groupIdParsed) &&
                            byte.TryParse(groupAndCommand[1], out byte commandIdParsed)) {
                            CommandDefinition cmd = CommandRegistry.GetCommandById(groupIdParsed, commandIdParsed);
                            if (cmd != null) {
                                commands.Add(new CommandViewModel(cmd));
                            }
                            else {
                                commands.Add(new CommandViewModel(new CommandDefinition(
                                    groupIdParsed,
                                    commandIdParsed,
                                    $"Unknown {groupIdParsed}.{commandIdParsed}",
                                    CommandAccessType.ReadOnly,
                                    "Brak opisu")));
                            }
                        }
                    }
                }

                GetCommandsListDataGrid.ItemsSource = commands;
            }
            catch (Exception ex) {
                Debug.WriteLine($"Błąd podczas przetwarzania listy komend: {ex.Message}");
                DialogHelper.ShowMessage(this, $"Wystąpił błąd podczas pobierania listy komend: {ex.Message}");
            }
        }

        private async void GetBuildVersionButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetBuildVersion");
                string response = await _commandManager.ExecuteCommandAsync(command);
                GetBuildVersionLabel.Content = "v" + response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania wersji oprogramowania: {ex.Message}");
            }
        }

        private async void GetVerbosityLevelButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("Verbosity");
                string response = await _commandManager.ExecuteCommandAsync(command);

                if (int.TryParse(response, out int verbosityLevel)) {
                    GetVerbosityLevelLabel.Content = $"{verbosityLevel} - {GetVerbosityLevelName(verbosityLevel)}";
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania poziomu szczegółowości: {ex.Message}");
            }
        }

        private async void SetVerbosityLevel_Click(object sender, RoutedEventArgs e) {
            try {
                if (SetVerbosityComboBox.SelectedItem is ComboBoxItem selectedItem) {
                    string verbosityLevel = selectedItem.Tag.ToString();
                    CommandDefinition command = CommandRegistry.GetCommandByName("Verbosity");
                    string response = await _commandManager.ExecuteCommandAsync(command, verbosityLevel, "SET");

                    DialogHelper.ShowMessage(this, $"Poziom szczegółowości został ustawiony pomyślnie. {response}");
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd ustawiania poziomu szczegółowości: {ex.Message}");
            }
        }


        private async void EnterBootloaderButton_Click(object sender, RoutedEventArgs e) {
            try {
                string password = await PromptForPasswordAsync();

                if (password == "xd") {
                    CommandDefinition command = CommandRegistry.GetCommandByName("EnterBootloader");
                    string response = await _commandManager.ExecuteCommandAsync(command, "USB", "SET");

                    DialogHelper.ShowMessage(this, "Bootloader uruchomiony pomyślnie.");
                }
                else {
                    DialogHelper.ShowMessage(this, "Nieprawidłowe hasło. Operacja anulowana.");
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd uruchamiania bootloadera: {ex.Message}");
            }
        }

        private async Task<string> PromptForPasswordAsync() {
            PasswordDialog passwordDialog = new PasswordDialog();
            bool? result = await passwordDialog.ShowDialog<bool>(this);
            return result == true ? passwordDialog.Password : string.Empty;
        }

        private string GetVerbosityLevelName(int level) {
            switch (level) {
                case 0: return "SILENT";
                case 1: return "ERROR";
                case 2: return "WARNING";
                case 3: return "INFO";
                case 4: return "DEBUG";
                default: return "UNKNOWN";
            }
        }

        private async void GetSystemUptimeButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetSystemUptime");
                string response = await _commandManager.ExecuteCommandAsync(command);

                if (long.TryParse(response, out long uptime)) {
                    TimeSpan timeSpan = TimeSpan.FromSeconds(uptime);
                    GetSystemUptimeLabel.Content = timeSpan.ToString();
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania czasu pracy: {ex.Message}");
            }
        }

        private async void GetPowerModeButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetPowerMode");
                string response = await _commandManager.ExecuteCommandAsync(command);
                GetPowerModeLabel.Content = response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania trybu pracy: {ex.Message}");
            }
        }

        private async void GetHealthButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetHealth");
                string response = await _commandManager.ExecuteCommandAsync(command);
                GetHealthLabel.Text = response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania zdrowia: {ex.Message}");
            }
        }

        private async void GetImuStatusButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetImuStatus");
                string response = await _commandManager.ExecuteCommandAsync(command);
                GetImuStatusLabel.Content = response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd statusu IMU: {ex.Message}");
            }
        }

        private async void GetAccelButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetAccel");
                string response = await _commandManager.ExecuteCommandAsync(command);
                GetAccelLabel.Content = response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd accel: {ex.Message}");
            }
        }

        private async void GetGyroButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetGyro");
                string response = await _commandManager.ExecuteCommandAsync(command);
                GetGyroLabel.Content = response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd gyro: {ex.Message}");
            }
        }

        private async void FormatSdButton_Click(object sender, RoutedEventArgs e) {
            try {
                bool confirmed = await DialogHelper.ShowConfirmAsync(this,
                    "To trwale usuwa dane na karcie SD. Kontynuować? (tylko lab)",
                    "Format SD");
                if (!confirmed)
                    return;

                CommandDefinition command = CommandRegistry.GetCommandByName("FormatSd");
                string response = await _commandManager.ExecuteCommandAsync(command, "FORMAT", "SET");
                DialogHelper.ShowMessage(this, $"Format SD: {response}");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd formatowania SD: {ex.Message}");
            }
        }

        #endregion
    }
}