namespace DGBackendApi.Entities;

public class GameSession
{
    public string SessionId { get; set; } = string.Empty;

    public long OwnerAccountId { get; set; }

    public long CurrentLeaderAccountId { get; set; }

    public string RegionId { get; set; } = string.Empty;

    public string RoomName { get; set; } = "";

    public string RoomPasswordHash { get; set; } = "";

    public string MapPath { get; set; } = string.Empty;

    public string ServerIp { get; set; } = string.Empty;

    public int ServerPort { get; set; }
    
    public int? ServerProcessId { get; set; }
    
    public string ServerRuntimeStatus { get; set; } = "None";

    public string Status { get; set; } = "Open";

    public int MaxPlayers { get; set; } = 4;

    public DateTime CreatedAtUtc { get; set; } = DateTime.UtcNow;

    public DateTime? StartedAtUtc { get; set; }

    public DateTime? LastHeartbeatAtUtc { get; set; }

    public DateTime? EndedAtUtc { get; set; }

    public List<SessionMember> Members { get; set; } = new();
}