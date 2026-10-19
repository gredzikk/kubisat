using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;


namespace kubisat_operator {
    public class EventLogEntry {
        public int EventId { get; set; }
        public DateTime EventTimestamp { get; set; }
        public int EventGroupId { get; set; }
        public int EventCode { get; set; }
        public string EventDescription { get; set; }
    }

    public partial class MainWindow : Window {
        public ObservableCollection<EventLogEntry> EventLogEntries { get; set; } = new ObservableCollection<EventLogEntry>();

        public ObservableCollection<CommandViewModel> EventCommands { get; set; }

        #region Event Commands
        private async void GetTotalEventCountButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetEventCount");
                string response = await _commandManager.ExecuteCommandAsync(command);

                GetTotalEventCountLabel.Content = response;
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania liczby zdarzeń: {ex.Message}");
            }
        }

        private async void GetAllEventsButton_Click(object sender, RoutedEventArgs e) {
            try {
                CommandDefinition command = CommandRegistry.GetCommandByName("GetLastEvents");
                string response = await _commandManager.ExecuteCommandAsync(command, "0");

                ProcessEventsResponse(response);
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania wszystkich zdarzeń: {ex.Message}");
            }
        }

        private async void GetLastEventsButton_Click(object sender, RoutedEventArgs e) {
            try {
                if (GetLastEventsCount.Value.HasValue) {
                    int eventCount = (int)GetLastEventsCount.Value.Value;

                    CommandDefinition command = CommandRegistry.GetCommandByName("GetLastEvents");
                    string response = await _commandManager.ExecuteCommandAsync(command, eventCount.ToString());

                    ProcessEventsResponse(response);
                }
                else {
                    DialogHelper.ShowMessage(this, "Wprowadź prawidłową liczbę zdarzeń do pobrania.");
                }
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Błąd pobierania ostatnich zdarzeń: {ex.Message}");
            }
        }

        private void ProcessEventsResponse(string value) {
            EventLogEntries.Clear();
            value = value.Replace(";", "");
            string[] events = value.Split('-');
            events = events.Reverse().ToArray();

            foreach (string ev in events) {
                if (ev.Length == 16) {
                    string part1 = ev.Substring(0, 4);
                    string part2 = ev.Substring(4, 8);
                    string part3 = ev.Substring(12, 2);
                    string part4 = ev.Substring(14, 2);

                    int eventId = Convert.ToInt32(part1, 16);
                    int timestamp = Convert.ToInt32(part2, 16);
                    int groupId = Convert.ToInt32(part3, 16);
                    int eventCode = Convert.ToInt32(part4, 16);

                    DateTime eventTime = DateTimeOffset.FromUnixTimeSeconds(timestamp).DateTime;

                    string description = GetEventDescription(groupId, eventCode);

                    EventLogEntry eventLogEntry = new EventLogEntry {
                        EventId = eventId,
                        EventTimestamp = eventTime,
                        EventGroupId = groupId,
                        EventCode = eventCode,
                        EventDescription = description
                    };

                    EventLogEntries.Add(eventLogEntry);
                }
                else {
                    Debug.WriteLine($"Nieprawidłowy format zdarzenia: {ev}");
                }
            }
        }

        private string GetEventDescription(int groupId, int eventCode) {
            switch (groupId) {
                case 0x00: // SYSTEM
                    switch (eventCode) {
                        case 0x01:
                            return "Uruchomienie systemu";
                        case 0x02:
                            return "Zatrzymanie systemu";
                        case 0x03:
                            return "Reset watchdog";
                        case 0x04:
                            return "Start rdzenia 1";
                        case 0x05:
                            return "Stop rdzenia 1";
                        default:
                            return $"Nieznane zdarzenie systemowe (kod: 0x{eventCode:X2})";
                    }
                case 0x01: // POWER
                    switch (eventCode) {
                        case 0x01:
                            return "Niski poziom baterii";
                        case 0x02:
                            return "Przeładowanie";
                        case 0x03:
                            return "Spadek zasilania";
                        case 0x04:
                            return "Powrót zasilania do normy";
                        case 0x05:
                            return "Rozpoczęto ładowanie z ogniw";
                        case 0x06:
                            return "Zakończono ładowanie z ogniw";
                        case 0x07:
                            return "Rozpoczęto ładowanie USB";
                        case 0x08:
                            return "Zakończono ładowanie USB";
                        case 0x09:
                            return "Zmiana bilansu prądów - ujemny";
                        case 0x0A:
                            return "Zmiana bilansu prądów - dodatni";
                        default:
                            return $"Nieznane zdarzenie zasilania (kod: 0x{eventCode:X2})";
                    }
                case 0x02: // COMMS
                    switch (eventCode) {
                        case 0x01:
                            return "Inicjalizacja radia";
                        case 0x02:
                            return "Błąd radia";
                        case 0x03:
                            return "Odebrano wiadomość";
                        case 0x04:
                            return "Wysłano wiadomość";
                        case 0x06:
                            return "Błąd UART";
                        default:
                            return $"Nieznane zdarzenie komunikacyjne (kod: 0x{eventCode:X2})";
                    }
                case 0x03: // GPS
                    switch (eventCode) {
                        case 0x01:
                            return "Uzyskano fix GPS";
                        case 0x02:
                            return "Utracono fix GPS";
                        case 0x03:
                            return "Błąd GPS";
                        case 0x04:
                            return "GPS włączony";
                        case 0x05:
                            return "GPS wyłączony";
                        case 0x06:
                            return "Dane GPS gotowe";
                        case 0x07:
                            return "Rozpoczęto tryb passthrough GPS";
                        case 0x08:
                            return "Zakończono tryb passthrough GPS";
                        default:
                            return $"Nieznane zdarzenie GPS (kod: 0x{eventCode:X2})";
                    }
                case 0x04: // CLOCK
                    switch (eventCode) {
                        case 0x01:
                            return "Zegar zmieniony";
                        case 0x02:
                            return "Synchronizacja zegara z GPS";
                        case 0x03:
                            return "Brak danych do synchronizacji";
                        default:
                            return $"Nieznane zdarzenie zegara (kod: 0x{eventCode:X2})";
                    }
                default:
                    return $"Nieznana grupa zdarzeń (grupa: 0x{groupId:X2})";
            }
        }

        private async void ExportEventLogToCsv(object sender, RoutedEventArgs e) {
            if (EventLogEntries == null || EventLogEntries.Count == 0) {
                DialogHelper.ShowMessage(this, "Brak danych do eksportu.");
                return;
            }

            string? fileName = await DialogHelper.SaveFileAsync(this, "Eksport do CSV", "LogZdarzen.csv", DialogHelper.CsvFileType);
            if (fileName == null) return;

            try {
                using (StreamWriter sw = new StreamWriter(fileName, false, Encoding.UTF8)) {
                    sw.WriteLine("EventId;EventTimestamp;EventGroupId;EventCode;EventDescription");

                    foreach (EventLogEntry logEntry in EventLogEntries) {
                        string line = $"{logEntry.EventId};{logEntry.EventTimestamp:yyyy-MM-dd HH:mm:ss};{logEntry.EventGroupId};{logEntry.EventCode};\"{logEntry.EventDescription}\"";
                        sw.WriteLine(line);
                    }
                }

                DialogHelper.ShowMessage(this, "Log zdarzeń został pomyślnie wyeksportowany.");
            }
            catch (Exception ex) {
                DialogHelper.ShowMessage(this, $"Wystąpił błąd podczas eksportu: {ex.Message}");
            }
        }

        #endregion
    }
}