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
 * - 생성한 JoinToken을 DB와 응답에 동일하게 사용
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
    var joinToken = $"local-token-{Guid.NewGuid():N}";

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
        JoinToken = joinToken,
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
        JoinToken: joinToken
    ));
});

/**
 * 세션 합류 API
 *
 * DB 저장 흐름:
 * - sessions 테이블에서 세션 조회
 * - session_members 테이블에 Member 저장 또는 기존 멤버 갱신
 * - 생성한 JoinToken을 DB와 응답에 동일하게 사용
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
    var joinToken = $"local-token-{Guid.NewGuid():N}";

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
        existingMember.JoinToken = joinToken;
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
            JoinToken = joinToken,
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
        JoinToken: joinToken
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

/**
 * 세션 접속 토큰 검증 API
 *
 * Dedicated Server가 클라이언트 접속 시 전달받은
 * SessionId / JoinToken을 Backend에 검증 요청할 때 사용한다.
 */
app.MapPost("/api/sessions/validate-join", async (
    ValidateJoinRequest request,
    DGDbContext db
) =>
{
    if (string.IsNullOrWhiteSpace(request.SessionId))
    {
        return Results.BadRequest(new ValidateJoinResponse(
            Success: false,
            SessionId: "",
            AccountId: 0,
            CharacterId: 0,
            Role: "",
            Message: "SessionId is required."
        ));
    }

    if (string.IsNullOrWhiteSpace(request.JoinToken))
    {
        return Results.BadRequest(new ValidateJoinResponse(
            Success: false,
            SessionId: request.SessionId,
            AccountId: 0,
            CharacterId: 0,
            Role: "",
            Message: "JoinToken is required."
        ));
    }

    var session = await db.Sessions
        .Include(x => x.Members)
        .FirstOrDefaultAsync(x => x.SessionId == request.SessionId);

    if (session == null)
    {
        return Results.NotFound(new ValidateJoinResponse(
            Success: false,
            SessionId: request.SessionId,
            AccountId: 0,
            CharacterId: 0,
            Role: "",
            Message: "Session not found."
        ));
    }

    if (session.Status != "Open" && session.Status != "Playing")
    {
        return Results.BadRequest(new ValidateJoinResponse(
            Success: false,
            SessionId: request.SessionId,
            AccountId: 0,
            CharacterId: 0,
            Role: "",
            Message: $"Session is not joinable. Current status: {session.Status}"
        ));
    }

    var member = session.Members.FirstOrDefault(x =>
        x.JoinToken == request.JoinToken &&
        x.Status == "Joined"
    );

    if (member == null)
    {
        return Results.BadRequest(new ValidateJoinResponse(
            Success: false,
            SessionId: request.SessionId,
            AccountId: 0,
            CharacterId: 0,
            Role: "",
            Message: "Invalid join token."
        ));
    }

    return Results.Ok(new ValidateJoinResponse(
        Success: true,
        SessionId: session.SessionId,
        AccountId: member.AccountId,
        CharacterId: member.CharacterId,
        Role: member.Role,
        Message: "Join token is valid."
    ));
});

/**
 * 세션 시작 보고 API
 *
 * Dedicated Server가 정상 JoinToken 검증 성공 후 호출한다.
 * sessions.status를 Playing으로 변경하고 started_at_utc를 기록한다.
 */
app.MapPost("/api/sessions/session-started", async (
    SessionStartedRequest request,
    DGDbContext db
) =>
{
    if (string.IsNullOrWhiteSpace(request.SessionId))
    {
        return Results.BadRequest(new SessionStartedResponse(
            Success: false,
            SessionId: "",
            Status: "",
            Message: "SessionId is required."
        ));
    }

    var session = await db.Sessions
        .FirstOrDefaultAsync(x => x.SessionId == request.SessionId);

    if (session == null)
    {
        return Results.NotFound(new SessionStartedResponse(
            Success: false,
            SessionId: request.SessionId,
            Status: "",
            Message: "Session not found."
        ));
    }

    if (session.Status == "Ended")
    {
        return Results.BadRequest(new SessionStartedResponse(
            Success: false,
            SessionId: session.SessionId,
            Status: session.Status,
            Message: "Session is already ended."
        ));
    }

    var now = DateTime.UtcNow;

session.Status = "Playing";

if (session.StartedAtUtc == null)
{
    session.StartedAtUtc = now;
}

session.LastHeartbeatAtUtc = now;

    await db.SaveChangesAsync();

    return Results.Ok(new SessionStartedResponse(
        Success: true,
        SessionId: session.SessionId,
        Status: session.Status,
        Message: "Session started."
    ));
});

/**
 * 세션 Heartbeat API
 *
 * Dedicated Server가 주기적으로 호출한다.
 * sessions.last_heartbeat_at_utc를 갱신한다.
 */
app.MapPost("/api/sessions/heartbeat", async (
    SessionHeartbeatRequest request,
    DGDbContext db
) =>
{
    if (string.IsNullOrWhiteSpace(request.SessionId))
    {
        return Results.BadRequest(new SessionHeartbeatResponse(
            Success: false,
            SessionId: "",
            Status: "",
            LastHeartbeatAtUtc: null,
            Message: "SessionId is required."
        ));
    }

    var session = await db.Sessions
        .FirstOrDefaultAsync(x => x.SessionId == request.SessionId);

    if (session == null)
    {
        return Results.NotFound(new SessionHeartbeatResponse(
            Success: false,
            SessionId: request.SessionId,
            Status: "",
            LastHeartbeatAtUtc: null,
            Message: "Session not found."
        ));
    }

    if (session.Status == "Ended")
    {
        return Results.BadRequest(new SessionHeartbeatResponse(
            Success: false,
            SessionId: session.SessionId,
            Status: session.Status,
            LastHeartbeatAtUtc: session.LastHeartbeatAtUtc,
            Message: "Session is already ended."
        ));
    }

    if (session.Status == "Open")
    {
        session.Status = "Playing";
    }

    session.LastHeartbeatAtUtc = DateTime.UtcNow;

    await db.SaveChangesAsync();

    return Results.Ok(new SessionHeartbeatResponse(
        Success: true,
        SessionId: session.SessionId,
        Status: session.Status,
        LastHeartbeatAtUtc: session.LastHeartbeatAtUtc,
        Message: "Heartbeat updated."
    ));
});

/**
 * 세션 종료 보고 API
 *
 * Dedicated Server가 종료될 때 호출한다.
 * sessions.status를 Ended로 변경하고 ended_at_utc를 기록한다.
 */
app.MapPost("/api/sessions/session-ended", async (
    SessionEndedRequest request,
    DGDbContext db
) =>
{
    if (string.IsNullOrWhiteSpace(request.SessionId))
    {
        return Results.BadRequest(new SessionEndedResponse(
            Success: false,
            SessionId: "",
            Status: "",
            EndedAtUtc: null,
            Message: "SessionId is required."
        ));
    }

    var session = await db.Sessions
        .FirstOrDefaultAsync(x => x.SessionId == request.SessionId);

    if (session == null)
    {
        return Results.NotFound(new SessionEndedResponse(
            Success: false,
            SessionId: request.SessionId,
            Status: "",
            EndedAtUtc: null,
            Message: "Session not found."
        ));
    }

    var now = DateTime.UtcNow;

    session.Status = "Ended";

    if (session.EndedAtUtc == null)
    {
        session.EndedAtUtc = now;
    }

    session.LastHeartbeatAtUtc = now;

    await db.SaveChangesAsync();

    return Results.Ok(new SessionEndedResponse(
        Success: true,
        SessionId: session.SessionId,
        Status: session.Status,
        EndedAtUtc: session.EndedAtUtc,
        Message: "Session ended."
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

public record ValidateJoinRequest(
    string SessionId,
    string JoinToken
);

public record ValidateJoinResponse(
    bool Success,
    string SessionId,
    long AccountId,
    long CharacterId,
    string Role,
    string Message
);

public record SessionStartedRequest(
    string SessionId
);

public record SessionStartedResponse(
    bool Success,
    string SessionId,
    string Status,
    string Message
);

public record SessionHeartbeatRequest(
    string SessionId
);

public record SessionHeartbeatResponse(
    bool Success,
    string SessionId,
    string Status,
    DateTime? LastHeartbeatAtUtc,
    string Message
);

public record SessionEndedRequest(
    string SessionId
);

public record SessionEndedResponse(
    bool Success,
    string SessionId,
    string Status,
    DateTime? EndedAtUtc,
    string Message
);