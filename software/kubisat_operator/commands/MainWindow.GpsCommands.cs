using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;
﻿using System.Collections.ObjectModel;

namespace kubisat_operator {
    public partial class MainWindow : Window {
        public ObservableCollection<CommandViewModel> GpsCommands { get; set; }

        private DispatcherTimer passthroughTimer;
        private DateTime passthroughEndTime;
        private double passthroughTotalDuration;

        #region GPS Commands
        private async void GetGpsPowerStatusButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("PowerStatus");
                string response = await _commandManager.ExecuteCommandAsync(command);

                if (response.Equals("1")) {
                    GetGpsPowerStatusLabel.Content = "WŁĄCZONE";
                    GetGpsPowerStatusLabel.Foreground = Brushes.Green;
                }
                else {
                    GetGpsPowerStatusLabel.Content = "WYŁĄCZONE";
                    GetGpsPowerStatusLabel.Foreground = Brushes.Red;
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania statusu zasilania GPS: {ex.Message}");
            }
        }

        private async void SetGpsPowerOffStatusButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("PowerStatus");
                string response = await _commandManager.ExecuteCommandAsync(command, "0", "SET");

                DialogHelper.ShowMessage(this, "Zasilanie GPS zostało wyłączone pomyślnie.");

                // Refresh the display
                await RefreshGpsPowerStatus();
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd wyłączania zasilania GPS: {ex.Message}");
            }
        }

        private async void SetGpsPowerOnStatusButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("PowerStatus");
                string response = await _commandManager.ExecuteCommandAsync(command, "1", "SET");

                DialogHelper.ShowMessage(this, "Zasilanie GPS zostało włączone pomyślnie.");

                // Refresh the display
                await RefreshGpsPowerStatus();
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd włączania zasilania GPS: {ex.Message}");
            }
        }

        // Helper method to refresh GPS power status
        private async Task RefreshGpsPowerStatus() {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("PowerStatus");
                string response = await _commandManager.ExecuteCommandAsync(command);

                if (response.Equals("1")) {
                    GetGpsPowerStatusLabel.Content = "WŁĄCZONE";
                    GetGpsPowerStatusLabel.Foreground = Brushes.Green;
                }
                else {
                    GetGpsPowerStatusLabel.Content = "WYŁĄCZONE";
                    GetGpsPowerStatusLabel.Foreground = Brushes.Red;
                }
            }
            catch (Exception) {
                // Silent failure for status refresh
                GetGpsPowerStatusLabel.Content = "NIEZNANY";
                GetGpsPowerStatusLabel.Foreground = Brushes.Orange;
            }
        }

        private async void StartGpsPassthroughModeButton_Click(object sender, RoutedEventArgs e) {
            try {
                if (GpsPassthroughModeDuration.Value.HasValue) {
                    int duration = (int)GpsPassthroughModeDuration.Value.Value;

                    CommandDefinition command = CommandRegistry.GetCommandByName("UartPassthrough");
                    string response = await _commandManager.ExecuteCommandAsync(command, duration.ToString(), "SET");

                    DialogHelper.ShowMessage(this, "Tryb przezroczysty GPS został uruchomiony pomyślnie.");

                    passthroughTotalDuration = duration;
                    passthroughEndTime = DateTime.Now.AddSeconds(duration);

                    if (passthroughTimer == null) {
                        passthroughTimer = new DispatcherTimer();
                        passthroughTimer.Interval = TimeSpan.FromSeconds(1);
                        passthroughTimer.Tick += PassthroughTimer_Tick;
                    }
                    else {
                        passthroughTimer.Stop();
                    }

                    passthroughTimer.Start();

                    GpsPassthroughModeDurationRemainingProgressBar.Maximum = 100;
                    GpsPassthroughModeDurationRemainingProgressBar.Value = 100;

                    GpsPassthroughModeDurationRemainingLabel.Content = $"100% ({duration}s pozostało)";
                }
                else {
                    DialogHelper.ShowMessage(this, "Wprowadź prawidłowy czas trwania trybu przezroczystego.");
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd uruchamiania trybu przezroczystego GPS: {ex.Message}");
            }
        }

        private void PassthroughTimer_Tick(object sender, EventArgs e) {
            double remainingTime = (passthroughEndTime - DateTime.Now).TotalSeconds;
            if (remainingTime <= 0) {
                passthroughTimer.Stop();
                GpsPassthroughModeDurationRemainingProgressBar.Value = 0;
                GpsPassthroughModeDurationRemainingLabel.Content = "Tryb przezroczysty zakończony.";
            }
            else {
                double percentage = remainingTime / passthroughTotalDuration * 100.0;
                GpsPassthroughModeDurationRemainingProgressBar.Value = percentage;
                GpsPassthroughModeDurationRemainingLabel.Content = $"{percentage:F0}% ({remainingTime:F0}s pozostało)";
            }
        }

        #endregion

    }
}
