using System.Diagnostics;
using System.Net.NetworkInformation;
using DGBackendApi.Data;
using DGBackendApi.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Options;

namespace DGBackendApi.Services;

public class DedicatedServerLaunchResult
{
    public bool Success { get; set; }

    public string Message { get; set; } = string.Empty;

    public int ServerPort { get; set; }

    public int? ProcessId { get; set; }
}

public class DedicatedServerLauncherService
{
    private readonly DGDbContext _db;
    private readonly DedicatedServerOptions _options;

    public DedicatedServerLauncherService(
        DGDbContext db,
        IOptions<DedicatedServerOptions> options
    )
    {
        _db = db;
        _options = options.Value;
    }

    public async Task<DedicatedServerLaunchResult> LaunchAsync(string sessionId)
    {
        if (string.IsNullOrWhiteSpace(sessionId))
        {
            return new DedicatedServerLaunchResult
            {
                Success = false,
                Message = "SessionId is required."
            };
        }

        if (string.IsNullOrWhiteSpace(_options.ServerExePath))
        {
            return new DedicatedServerLaunchResult
            {
                Success = false,
                Message = "ServerExePath is empty."
            };
        }

        if (!File.Exists(_options.ServerExePath))
        {
            return new DedicatedServerLaunchResult
            {
                Success = false,
                Message = $"ServerExePath not found. Path={_options.ServerExePath}"
            };
        }

        if (string.IsNullOrWhiteSpace(_options.MapPath))
        {
            return new DedicatedServerLaunchResult
            {
                Success = false,
                Message = "MapPath is empty."
            };
        }

        if (string.IsNullOrWhiteSpace(_options.BackendBaseUrl))
        {
            return new DedicatedServerLaunchResult
            {
                Success = false,
                Message = "BackendBaseUrl is empty."
            };
        }

        var serverPort = await FindAvailablePortAsync();

        if (serverPort <= 0)
        {
            return new DedicatedServerLaunchResult
            {
                Success = false,
                Message = "No available server port."
            };
        }

        var logDirectory = Path.Combine(
    Directory.GetCurrentDirectory(),
    "Logs",
    "DedicatedServers"
);

Directory.CreateDirectory(logDirectory);

var logFilePath = Path.Combine(
    logDirectory,
    $"Dedi_{serverPort}_{sessionId}.log"
);

var arguments =
    $"{_options.MapPath} -server -log -FORCELOGFLUSH -AbsLog=\"{logFilePath}\" -port={serverPort} -SessionId={sessionId} -BackendUrl={_options.BackendBaseUrl}";

        var startInfo = new ProcessStartInfo
        {
            FileName = _options.ServerExePath,
            Arguments = arguments,
            WorkingDirectory = Path.GetDirectoryName(_options.ServerExePath) ?? "",
            UseShellExecute = false,
            CreateNoWindow = false
        };

        try
        {
            var process = Process.Start(startInfo);

            if (process == null)
            {
                return new DedicatedServerLaunchResult
                {
                    Success = false,
                    Message = "Failed to start dedicated server process.",
                    ServerPort = serverPort
                };
            }

            return new DedicatedServerLaunchResult
            {
                Success = true,
                Message = "Dedicated server process started.",
                ServerPort = serverPort,
                ProcessId = process.Id
            };
        }
        catch (Exception ex)
        {
            return new DedicatedServerLaunchResult
            {
                Success = false,
                Message = $"Failed to launch dedicated server. {ex.Message}",
                ServerPort = serverPort
            };
        }
    }

    private async Task<int> FindAvailablePortAsync()
    {
        var usedPortsFromDb = await _db.Sessions
            .Where(x =>
                x.Status != "Ended" &&
                x.ServerRuntimeStatus != "Exited" &&
                x.ServerRuntimeStatus != "Failed" &&
                x.ServerPort >= _options.MinPort &&
                x.ServerPort <= _options.MaxPort
            )
            .Select(x => x.ServerPort)
            .ToListAsync();

        var usedPortsFromOs = GetUsedPorts();

        for (var port = _options.MinPort; port <= _options.MaxPort; port++)
        {
            if (usedPortsFromDb.Contains(port))
            {
                continue;
            }

            if (usedPortsFromOs.Contains(port))
            {
                continue;
            }

            return port;
        }

        return -1;
    }

    private static HashSet<int> GetUsedPorts()
    {
        var properties = IPGlobalProperties.GetIPGlobalProperties();

        var activeTcpListeners = properties
            .GetActiveTcpListeners()
            .Select(x => x.Port);

        var activeTcpConnections = properties
            .GetActiveTcpConnections()
            .Select(x => x.LocalEndPoint.Port);

        var activeUdpListeners = properties
            .GetActiveUdpListeners()
            .Select(x => x.Port);

        return activeTcpListeners
            .Concat(activeTcpConnections)
            .Concat(activeUdpListeners)
            .ToHashSet();
    }
}