using System.ComponentModel;
using Avalonia.Controls;

namespace kubisat_operator;

public class CommandViewModel : INotifyPropertyChanged {
    public CommandDefinition Command { get; private set; }
    public string Name => Command.Name;
    public string DisplayName { get; set; }
    public CommandAccessType Access => Command.Access;
    public byte GroupId => Command.GroupId;
    public byte Id => Command.Id;
    public string CommandDescription => Command.Description;

    private string _value = string.Empty;
    public string Value {
        get { return _value; }
        set {
            if (_value != value) {
                _value = value;
                OnPropertyChanged(nameof(Value));
            }
        }
    }

    public bool IsReadOnlyVisible => Command.Access == CommandAccessType.ReadOnly;
    public bool IsWriteOnlyVisible => Command.Access == CommandAccessType.WriteOnly;
    public bool IsReadWriteVisible => Command.Access == CommandAccessType.ReadWrite;

    public string OutputType { get; set; }

    public CommandViewModel(CommandDefinition command) {
        Command = command;
        DisplayName = command.Name;
        OutputType = "TextBlock";
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    protected virtual void OnPropertyChanged(string propertyName) {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}
