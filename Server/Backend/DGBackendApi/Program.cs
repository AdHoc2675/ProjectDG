using DGBackendApi.Data;
using DGBackendApi.Entities;
using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddDbContext<DGDbContext>(options =>
{
    var connectionString = builder.Configuration.GetConnectionString("ProjectDGDatabase");
    options.UseNpgsql(connectionString);
});

var app = builder.Build();

app.MapGet("/", () =>
{
    return Results.Ok(new
    {
        success = true,
        message = "DG Backend API is running."
    });
});

app.MapGet("/health", () =>
{
    return Results.Ok(new
    {
        success = true,
        service = "DGBackendApi",
        status = "Healthy"
    });
});

/**
 * 세션 생성 API
 *
 * DB 저장 흐름:
 * - sessions 테이블에 세션 저장
 * - session_members 테이블에 Leader 멤버 저장
 */
app.MapPost("/api/sessions/create", async (
    CreateSessionRequest request,
    DGDbContext db
) =>
{
    if (request.AccountId <= 0)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "AccountId must be greater than 0."
        });
    }

    if (request.CharacterId <= 0)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "CharacterId must be greater than 0."
        });
    }

    if (string.IsNullOrWhiteSpace(request.RegionId))
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "RegionId is required."
        });
    }

    var sessionId = $"local-session-{Guid.NewGuid():N}";

    var session = new GameSession
    {
        SessionId = sessionId,
        OwnerAccountId = request.AccountId,
        CurrentLeaderAccountId = request.AccountId,
        RegionId = request.RegionId,
        MapPath = "/Game/Personal/DOHEE/Level/ServerTest",
        ServerIp = "127.0.0.1",
        ServerPort = 7777,
        Status = "Open",
        MaxPlayers = 4,
        CreatedAtUtc = DateTime.UtcNow
    };

    var leaderMember = new SessionMember
    {
        SessionId = sessionId,
        AccountId = request.AccountId,
        CharacterId = request.CharacterId,
        Role = "Leader",
        Status = "Joined",
        JoinedAtUtc = DateTime.UtcNow
    };

    session.Members.Add(leaderMember);

    db.Sessions.Add(session);

    await db.SaveChangesAsync();

    return Results.Ok(new CreateSessionResponse(
        Success: true,
        SessionId: session.SessionId,
        ServerIp: session.ServerIp,
        ServerPort: session.ServerPort,
        MapPath: session.MapPath,
        JoinToken: $"local-token-{Guid.NewGuid():N}"
    ));
});

/**
 * 세션 합류 API
 *
 * DB 저장 흐름:
 * - sessions 테이블에서 세션 조회
 * - session_members 테이블에 Member 저장
 */
app.MapPost("/api/sessions/join", async (
    JoinSessionRequest request,
    DGDbContext db
) =>
{
    if (string.IsNullOrWhiteSpace(request.SessionId))
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "SessionId is required."
        });
    }

    if (request.AccountId <= 0)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "AccountId must be greater than 0."
        });
    }

    if (request.CharacterId <= 0)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "CharacterId must be greater than 0."
        });
    }

    var session = await db.Sessions
        .Include(x => x.Members)
        .FirstOrDefaultAsync(x => x.SessionId == request.SessionId);

    if (session == null)
    {
        return Results.NotFound(new
        {
            success = false,
            message = "Session not found."
        });
    }

    if (session.Status != "Open" && session.Status != "Playing")
    {
        return Results.BadRequest(new
        {
            success = false,
            message = $"Session is not joinable. Current status: {session.Status}"
        });
    }

    var joinedMemberCount = session.Members.Count(x => x.Status == "Joined");
    var existingMember = session.Members.FirstOrDefault(x => x.CharacterId == request.CharacterId);

    if (existingMember == null && joinedMemberCount >= session.MaxPlayers)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "Session is full."
        });
    }

    if (existingMember != null)
    {
        existingMember.AccountId = request.AccountId;
        existingMember.Status = "Joined";
        existingMember.JoinedAtUtc = DateTime.UtcNow;
        existingMember.LeftAtUtc = null;

        if (existingMember.Role != "Leader")
        {
            existingMember.Role = "Member";
        }
    }
    else
    {
        var member = new SessionMember
        {
            SessionId = session.SessionId,
            AccountId = request.AccountId,
            CharacterId = request.CharacterId,
            Role = "Member",
            Status = "Joined",
            JoinedAtUtc = DateTime.UtcNow
        };

        db.SessionMembers.Add(member);
    }

    await db.SaveChangesAsync();

    return Results.Ok(new JoinSessionResponse(
        Success: true,
        SessionId: session.SessionId,
        ServerIp: session.ServerIp,
        ServerPort: session.ServerPort,
        MapPath: session.MapPath,
        JoinToken: $"local-token-{Guid.NewGuid():N}"
    ));
});

/**
 * 세션 조회 API
 *
 * DB 조회 흐름:
 * - sessions 테이블에서 세션 조회
 * - session_members 테이블까지 Include
 */
app.MapGet("/api/sessions/{sessionId}", async (
    string sessionId,
    DGDbContext db
) =>
{
    if (string.IsNullOrWhiteSpace(sessionId))
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "SessionId is required."
        });
    }

    var session = await db.Sessions
        .Include(x => x.Members)
        .FirstOrDefaultAsync(x => x.SessionId == sessionId);

    if (session == null)
    {
        return Results.NotFound(new
        {
            success = false,
            message = "Session not found."
        });
    }

    var members = session.Members
        .OrderBy(x => x.Id)
        .Select(x => new SessionMemberResponse(
            Id: x.Id,
            AccountId: x.AccountId,
            CharacterId: x.CharacterId,
            Role: x.Role,
            Status: x.Status,
            JoinedAtUtc: x.JoinedAtUtc,
            LeftAtUtc: x.LeftAtUtc
        ))
        .ToList();

    return Results.Ok(new SessionDetailResponse(
        Success: true,
        SessionId: session.SessionId,
        OwnerAccountId: session.OwnerAccountId,
        CurrentLeaderAccountId: session.CurrentLeaderAccountId,
        RegionId: session.RegionId,
        MapPath: session.MapPath,
        ServerIp: session.ServerIp,
        ServerPort: session.ServerPort,
        Status: session.Status,
        MaxPlayers: session.MaxPlayers,
        CurrentPlayers: members.Count(x => x.Status == "Joined"),
        Members: members
    ));
});

app.Run("http://localhost:8080");

public record CreateSessionRequest(
    long AccountId,
    long CharacterId,
    string RegionId
);

public record CreateSessionResponse(
    bool Success,
    string SessionId,
    string ServerIp,
    int ServerPort,
    string MapPath,
    string JoinToken
);

public record JoinSessionRequest(
    string SessionId,
    long AccountId,
    long CharacterId
);

public record JoinSessionResponse(
    bool Success,
    string SessionId,
    string ServerIp,
    int ServerPort,
    string MapPath,
    string JoinToken
);

public record SessionDetailResponse(
    bool Success,
    string SessionId,
    long OwnerAccountId,
    long CurrentLeaderAccountId,
    string RegionId,
    string MapPath,
    string ServerIp,
    int ServerPort,
    string Status,
    int MaxPlayers,
    int CurrentPlayers,
    List<SessionMemberResponse> Members
);

public record SessionMemberResponse(
    long Id,
    long AccountId,
    long CharacterId,
    string Role,
    string Status,
    DateTime JoinedAtUtc,
    DateTime? LeftAtUtc
);