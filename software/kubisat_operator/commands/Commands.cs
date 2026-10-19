namespace kubisat_operator {

    /// <summary>
    /// Defines the access type for commands.
    /// </summary>
    public enum CommandAccessType {
        ReadOnly,
        WriteOnly,
        ReadWrite
    }

    /// <summary>
    /// Represents a command definition with its properties and access rights.
    /// </summary>
    public class CommandDefinition {
        /// <summary>
        /// Gets the group ID of the command.
        /// </summary>
        public byte GroupId { get; }

        /// <summary>
        /// Gets the unique identifier of the command within its group.
        /// </summary>
        public byte Id { get; }

        /// <summary>
        /// Gets the name of the command.
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// Gets the access type of the command.
        /// </summary>
        public CommandAccessType Access { get; }

        /// <summary>
        /// Gets the description of the command's functionality.
        /// </summary>
        public string Description { get; }

        /// <summary>
        /// Initializes a new instance of the CommandDefinition class.
        /// </summary>
        /// <param name="groupId">The group ID of the command.</param>
        /// <param name="id">The command ID within the group.</param>
        /// <param name="name">The name of the command.</param>
        /// <param name="access">The access type for the command.</param>
        /// <param name="description">The description of the command's functionality.</param>
        public CommandDefinition(byte groupId, byte id, string name, CommandAccessType access, string description) {
            GroupId = groupId;
            Id = id;
            Name = name;
            Access = access;
            Description = description;
        }
    }

    /// <summary>
    /// Represents a group of related commands.
    /// </summary>
    public class CommandGroup {
        /// <summary>
        /// Gets the unique identifier of the command group.
        /// </summary>
        public byte Id { get; }

        /// <summary>
        /// Gets the name of the command group.
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// Gets the description of the command group's purpose.
        /// </summary>
        public string Description { get; }

        /// <summary>
        /// Gets the list of commands in this group.
        /// </summary>
        public List<CommandDefinition> Commands { get; }

        /// <summary>
        /// Initializes a new instance of the CommandGroup class.
        /// </summary>
        /// <param name="id">The group identifier.</param>
        /// <param name="name">The name of the group.</param>
        /// <param name="description">The description of the group's purpose.</param>
        /// <param name="commands">The list of commands in this group.</param>
        public CommandGroup(byte id, string name, string description, List<CommandDefinition> commands) {
            Id = id;
            Name = name;
            Description = description;
            Commands = commands;
        }
    }


    /// <summary>
    /// Manages the registration and lookup of commands and command groups.
    /// </summary>
    public static class CommandRegistry {
        private static readonly Dictionary<string, CommandDefinition> _commandsByName = new();
        private static readonly Dictionary<string, CommandDefinition> _commandsById = new();
        private static readonly List<CommandGroup> _groups = new();

        /// <summary>
        /// Gets a read-only list of all registered command groups.
        /// </summary>
        public static IReadOnlyList<CommandGroup> Groups => _groups.AsReadOnly();

        /// <summary>
        /// Initializes the command registry by registering all command groups.
        /// </summary>
        public static void Initialize() {
            RegisterDiagnosticCommands();
            RegisterImuCommands();
            RegisterClockCommands();
            RegisterEventCommands();
            RegisterGpsCommands();
            RegisterTelemetryCommands();
        }

        /// <summary>
        /// Retrieves a command definition by its name.
        /// </summary>
        /// <param name="name">The name of the command.</param>
        /// <returns>The command definition if found; otherwise, null.</returns>
        public static CommandDefinition GetCommandByName(string name) {
            return _commandsByName.TryGetValue(name, out CommandDefinition? command) ? command : null;
        }

        /// <summary>
        /// Retrieves a command definition by its group and command IDs.
        /// </summary>
        /// <param name="groupId">The group ID.</param>
        /// <param name="id">The command ID.</param>
        /// <returns>The command definition if found; otherwise, null.</returns>
        public static CommandDefinition GetCommandById(byte groupId, byte id) {
            return _commandsById.TryGetValue($"{groupId}-{id}", out CommandDefinition? command) ? command : null;
        }

        /// <summary>
        /// Retrieves all commands belonging to a specific group.
        /// </summary>
        /// <param name="groupId">The group ID to filter by.</param>
        /// <returns>An enumerable collection of commands in the specified group.</returns>
        public static IEnumerable<CommandDefinition> GetCommandsByGroup(byte groupId) {
            return _commandsById.Values.Where(c => c.GroupId == groupId);
        }

        /// <summary>
        /// Registers a single command in the registry.
        /// </summary>
        /// <param name="command">The command to register.</param>
        private static void RegisterCommand(CommandDefinition command) {
            _commandsByName[command.Name] = command;
            _commandsById[$"{command.GroupId}-{command.Id}"] = command;
        }

        /// <summary>
        /// Registers a command group and all its commands in the registry.
        /// </summary>
        /// <param name="group">The command group to register.</param>
        private static void RegisterCommandGroup(CommandGroup group) {
            _groups.Add(group);
            foreach (CommandDefinition command in group.Commands) {
                RegisterCommand(command);
            }
        }

        private static void RegisterDiagnosticCommands() {
            List<CommandDefinition> commands = new List<CommandDefinition>
            {
                new CommandDefinition(1, 0, "GetCommandsList", CommandAccessType.ReadOnly, "List all available commands"),
                new CommandDefinition(1, 1, "GetBuildVersion", CommandAccessType.ReadOnly, "Get firmware build version"),
                new CommandDefinition(1, 2, "GetPowerMode", CommandAccessType.ReadOnly, "Get system power mode"),
                new CommandDefinition(1, 3, "GetSystemUptime", CommandAccessType.ReadOnly, "Get system uptime"),
                new CommandDefinition(1, 4, "GetHealth", CommandAccessType.ReadOnly, "Health summary UP,RAD,SD,ENV,LUX,IMU,BAT"),
                new CommandDefinition(1, 5, "FormatSd", CommandAccessType.WriteOnly, "Format SD card (lab only, payload FORMAT)"),
                new CommandDefinition(1, 8, "Verbosity", CommandAccessType.ReadWrite, "Get/Set UART verbosity level (0-4)"),
                new CommandDefinition(1, 9, "EnterBootloader", CommandAccessType.WriteOnly, "Reboot system to USB firmware loader")
            };

            RegisterCommandGroup(new CommandGroup(1, "Diagnostic", "System diagnostic commands", commands));
        }

        private static void RegisterImuCommands() {
            List<CommandDefinition> commands = new List<CommandDefinition> {
                new CommandDefinition(2, 0, "GetImuStatus", CommandAccessType.ReadOnly, "IMU init status"),
                new CommandDefinition(2, 1, "GetAccel", CommandAccessType.ReadOnly, "Accelerometer ax,ay,az (g)"),
                new CommandDefinition(2, 2, "GetGyro", CommandAccessType.ReadOnly, "Gyroscope gx,gy,gz (dps)")
            };
            RegisterCommandGroup(new CommandGroup(2, "IMU", "MPU-6050 IMU commands", commands));
        }

        private static void RegisterClockCommands() {
            List<CommandDefinition> commands = new List<CommandDefinition> {
                new CommandDefinition(3, 0, "Time", CommandAccessType.ReadWrite, "Get/Set system time"),
                new CommandDefinition(3, 1, "TimezoneOffset", CommandAccessType.ReadWrite, "Get/Set timezone offset"),
                new CommandDefinition(3, 4, "GetInternalTemp", CommandAccessType.ReadOnly, "Get clock internal temperature in C")
            };
            RegisterCommandGroup(new CommandGroup(3, "Clock", "Clock and time management commands", commands));
        }
        private static void RegisterEventCommands() {
            List<CommandDefinition> commands = new List<CommandDefinition> {
                new CommandDefinition(5, 1, "GetLastEvents", CommandAccessType.ReadOnly, "Get last N events from log"),
                new CommandDefinition(5, 2, "GetEventCount", CommandAccessType.ReadOnly, "Get total number of events in log")
            };
            RegisterCommandGroup(new CommandGroup(5, "Events", "Event log commands", commands));
        }
        private static void RegisterGpsCommands() {
            List<CommandDefinition> commands = new List<CommandDefinition> {
                new CommandDefinition(7, 1, "PowerStatus", CommandAccessType.ReadWrite, "Get/Set GPS power state"),
                new CommandDefinition(7, 2, "UartPassthrough", CommandAccessType.WriteOnly, "Enable GPS UART passthrough"),
            };
            RegisterCommandGroup(new CommandGroup(7, "GPS", "GPS commands", commands));
        }
        private static void RegisterTelemetryCommands() {
            List<CommandDefinition> commands = new List<CommandDefinition> {
                new CommandDefinition(8, 2, "GetLastRecord", CommandAccessType.ReadOnly, "Get last telemetry record"),
                new CommandDefinition(8, 3, "GetLastSensorData", CommandAccessType.ReadOnly, "Get last sensor data"),
                new CommandDefinition(8, 4, "GetRecentTelemetry", CommandAccessType.ReadOnly, "Get recent N telemetry records from RAM"),
            };
            RegisterCommandGroup(new CommandGroup(8, "Telemetry", "Telemetry data commands", commands));
        }
    }
}













