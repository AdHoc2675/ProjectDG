using System.Collections.Concurrent;

var builder = WebApplication.CreateBuilder(args);

var app = builder.Build();

/**
 * 임시 메모리 세션 저장소
 *
 * 지금은 DB를 아직 연결하지 않았기 때문에,
 * 서버가 켜져 있는 동안만 세션 정보를 메모리에 저장한다.
 *
 * 나중에는 PostgreSQL sessions 테이블로 교체한다.
 */
var sessions = new ConcurrentDictionary<string, SessionInfo>();

/**
 * 임시 메모리 세션 멤버 저장소
 *
 * Key 1: SessionId
 * Key 2: CharacterId
 */
var sessionMembers = new ConcurrentDictionary<string, ConcurrentDictionary<long, SessionMemberInfo>>();

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
 * 현재는 로컬 Dedicated Server 주소를 고정 반환한다.
 */
app.MapPost("/api/sessions/create", (CreateSessionRequest request) =>
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

    var session = new SessionInfo(
        SessionId: sessionId,
        OwnerAccountId: request.AccountId,
        CurrentLeaderAccountId: request.AccountId,
        RegionId: request.RegionId,
        MapPath: "/Game/Personal/DOHEE/Level/ServerTest",
        ServerIp: "127.0.0.1",
        ServerPort: 7777,
        Status: "Open",
        MaxPlayers: 4,
        CreatedAtUtc: DateTime.UtcNow
    );

    sessions[sessionId] = session;

    /**
     * 세션 생성자는 자동으로 Leader 멤버로 등록한다.
     */
    var members = new ConcurrentDictionary<long, SessionMemberInfo>();

    members[request.CharacterId] = new SessionMemberInfo(
        SessionId: sessionId,
        AccountId: request.AccountId,
        CharacterId: request.CharacterId,
        Role: "Leader",
        Status: "Joined",
        JoinedAtUtc: DateTime.UtcNow
    );

    sessionMembers[sessionId] = members;

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
 * 이미 생성된 SessionId를 기준으로 세션에 합류한다.
 */
app.MapPost("/api/sessions/join", (JoinSessionRequest request) =>
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

    if (!sessions.TryGetValue(request.SessionId, out var session))
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

    if (!sessionMembers.TryGetValue(request.SessionId, out var members))
    {
        members = new ConcurrentDictionary<long, SessionMemberInfo>();
        sessionMembers[request.SessionId] = members;
    }

    if (members.Count >= session.MaxPlayers)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "Session is full."
        });
    }

    /**
     * 같은 CharacterId가 이미 들어와 있으면 중복 합류로 보고,
     * 기존 접속 정보를 그대로 갱신한다.
     */
    members[request.CharacterId] = new SessionMemberInfo(
        SessionId: request.SessionId,
        AccountId: request.AccountId,
        CharacterId: request.CharacterId,
        Role: "Member",
        Status: "Joined",
        JoinedAtUtc: DateTime.UtcNow
    );

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
 * 특정 SessionId의 현재 상태를 조회한다.
 *
 * 예:
 * GET http://localhost:8080/api/sessions/local-session-xxxx
 */
app.MapGet("/api/sessions/{sessionId}", (string sessionId) =>
{
    if (string.IsNullOrWhiteSpace(sessionId))
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "SessionId is required."
        });
    }

    if (!sessions.TryGetValue(sessionId, out var session))
    {
        return Results.NotFound(new
        {
            success = false,
            message = "Session not found."
        });
    }

    var members = sessionMembers.TryGetValue(sessionId, out var foundMembers)
        ? foundMembers.Values.ToList()
        : new List<SessionMemberInfo>();

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
        CurrentPlayers: members.Count,
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

public record SessionInfo(
    string SessionId,
    long OwnerAccountId,
    long CurrentLeaderAccountId,
    string RegionId,
    string MapPath,
    string ServerIp,
    int ServerPort,
    string Status,
    int MaxPlayers,
    DateTime CreatedAtUtc
);

public record SessionMemberInfo(
    string SessionId,
    long AccountId,
    long CharacterId,
    string Role,
    string Status,
    DateTime JoinedAtUtc
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
    List<SessionMemberInfo> Members
);