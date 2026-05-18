namespace DGBackendApi.Entities;

public class SessionMember
{
    public long Id { get; set; }

    public string SessionId { get; set; } = string.Empty;

    public long AccountId { get; set; }

    public long CharacterId { get; set; }

    public string Role { get; set; } = "Member";

    public string Status { get; set; } = "Joined";

    public string JoinToken { get; set; } = string.Empty;

    public DateTime JoinedAtUtc { get; set; } = DateTime.UtcNow;

    public DateTime? LeftAtUtc { get; set; }

    public GameSession? Session { get; set; }
}