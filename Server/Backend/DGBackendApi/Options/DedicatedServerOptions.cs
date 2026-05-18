namespace DGBackendApi.Options;

public class DedicatedServerOptions
{
    public string ServerExePath { get; set; } = string.Empty;

    public string MapPath { get; set; } = string.Empty;

    public string PublicServerIp { get; set; } = string.Empty;

    public int MinPort { get; set; } = 7777;

    public int MaxPort { get; set; } = 7799;
}