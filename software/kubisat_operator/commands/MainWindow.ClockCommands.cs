using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;
﻿using System.Collections.ObjectModel;

namespace kubisat_operator {
    public partial class MainWindow : Window {

        /// <summary>
        /// Gets or sets the collection of clock-related commands available in the UI.
        /// </summary>
        public ObservableCollection<CommandViewModel> ClockCommands { get; set; }

        /// <summary>
        /// Handles the Get Time button click event.
        /// Retrieves the current system time from the satellite and displays it in the UI.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">Event data.</param>
        private async void GetTimeButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("Time");
                string response = await _commandManager.ExecuteCommandAsync(command);

                if (long.TryParse(response, out long unixTimestamp)) {
                    DateTime dateTime = DateTimeOffset.FromUnixTimeSeconds(unixTimestamp).DateTime;
                    GetTimeLabel.Content = dateTime.ToString();
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania czasu: {ex.Message}");
            }
        }

        /// <summary>
        /// Handles the Set Time button click event.
        /// Sets the system time on the satellite using the provided date and time.
        /// Expected format: "HH:mm:ss dd-MM-yyyy"
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">Event data.</param>
        private async void SetTimeButton_Click(object sender, RoutedEventArgs e) {
            try {
                if (DateTime.TryParseExact(SetTimeTextBox.Text, "HH:mm:ss dd-MM-yyyy", null,
                    System.Globalization.DateTimeStyles.None, out DateTime combinedDateTime)) {
                    long unixTimestamp = ((DateTimeOffset)combinedDateTime).ToUnixTimeSeconds();

                    CommandDefinition command = CommandRegistry.GetCommandByName("Time");
                    string response = await _commandManager.ExecuteCommandAsync(command, unixTimestamp.ToString(), "SET");

                    DialogHelper.ShowMessage(this, "Czas został ustawiony pomyślnie.");
                }
                else {
                    DialogHelper.ShowMessage(this, "Nieprawidłowy format czasu. Użyj HH:mm:ss dd-MM-yyyy.");
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd ustawiania czasu: {ex.Message}");
            }
        }

        /// <summary>
        /// Handles the Get Timezone Offset button click event.
        /// Retrieves the current timezone offset from the satellite.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">Event data.</param>
        private async void GetTimezoneOffsetButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("TimezoneOffset");
                string response = await _commandManager.ExecuteCommandAsync(command);

                GetTimezoneOffsetLabel.Content = response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania strefy czasowej: {ex.Message}");
            }
        }

        /// <summary>
        /// Handles the Set Timezone Offset button click event.
        /// Sets the timezone offset on the satellite.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">Event data.</param>
        private async void SetTimezoneOffsetButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("TimezoneOffset");
                string response = await _commandManager.ExecuteCommandAsync(command, SetTimezoneOffsetTextBox.Text, "SET");

                DialogHelper.ShowMessage(this, "Strefa czasowa została ustawiona pomyślnie.");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd ustawiania strefy czasowej: {ex.Message}");
            }
        }

        /// <summary>
        /// Handles the Get Clock Internal Temperature button click event.
        /// Retrieves the internal temperature of the satellite's clock module.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">Event data.</param>
        private async void GetClockInternalTemperatureButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetInternalTemp");
                string response = await _commandManager.ExecuteCommandAsync(command);

                GetClockInternalTemperatureLabel.Content = response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania temperatury zegara: {ex.Message}");
            }
        }
    }
}