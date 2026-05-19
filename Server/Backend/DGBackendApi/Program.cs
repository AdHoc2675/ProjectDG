using System.Security.Cryptography;
using System.Text;
using DGBackendApi.Data;
using DGBackendApi.Entities;
using DGBackendApi.Options;
using DGBackendApi.Services;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Options;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddDbContext<DGDbContext>(options =>
{
    var connectionString = builder.Configuration.GetConnectionString("ProjectDGDatabase");
    options.UseNpgsql(connectionString);
});

builder.Services.Configure<DedicatedServerOptions>(
    builder.Configuration.GetSection("DedicatedServer"));

builder.Services.AddScoped<DedicatedServerLauncherService>();

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
 * 유저 입력:
 * - RoomName
 * - RoomPassword
 *
 * 내부 처리:
 * - SessionId 생성
 * - JoinToken 생성
 * - RoomPassword는 Hash로만 저장
 * - Dedicated Server 프로세스 실행
 */
app.MapPost("/api/sessions/create", async (
    CreateSessionRequest request,
    DGDbContext db,
    DedicatedServerLauncherService serverLauncher,
    IOptions<DedicatedServerOptions> dedicatedServerOptions
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

    var roomName = NormalizeRoomName(request.RoomName);

    if (string.IsNullOrWhiteSpace(roomName))
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "RoomName is required."
        });
    }

    if (string.IsNullOrWhiteSpace(request.RoomPassword))
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "RoomPassword is required."
        });
    }

    var existingRoom = await db.Sessions
        .FirstOrDefaultAsync(x =>
            x.RoomName == roomName &&
            x.Status != "Ended"
        );

    if (existingRoom != null)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "Room name already exists."
        });
    }

    var sessionId = $"local-session-{Guid.NewGuid():N}";
    var joinToken = $"local-token-{Guid.NewGuid():N}";

    var launchResult = await serverLauncher.LaunchAsync(sessionId);

    if (!launchResult.Success)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = launchResult.Message
        });
    }

    var dedicatedServer = dedicatedServerOptions.Value;

    var session = new GameSession
    {
        SessionId = sessionId,
        OwnerAccountId = request.AccountId,
        CurrentLeaderAccountId = request.AccountId,
        RegionId = request.RegionId,
        RoomName = roomName,
        RoomPasswordHash = HashRoomPassword(request.RoomPassword),
        MapPath = dedicatedServer.MapPath,
        ServerIp = dedicatedServer.PublicServerIp,
        ServerPort = launchResult.ServerPort,
        ServerProcessId = launchResult.ProcessId,
        ServerRuntimeStatus = "Starting",
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
 * 유저 입력:
 * - RoomName
 * - RoomPassword
 *
 * 내부 처리:
 * - RoomName으로 Open/Playing 세션 조회
 * - RoomPassword Hash 검증
 * - 참가자용 JoinToken 새로 발급
 */
app.MapPost("/api/sessions/join", async (
    JoinSessionRequest request,
    DGDbContext db
) =>
{
    var roomName = NormalizeRoomName(request.RoomName);

    if (string.IsNullOrWhiteSpace(roomName))
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "RoomName is required."
        });
    }

    if (string.IsNullOrWhiteSpace(request.RoomPassword))
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "RoomPassword is required."
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
        .FirstOrDefaultAsync(x =>
            x.RoomName == roomName &&
            x.Status != "Ended"
        );

    if (session == null)
    {
        return Results.NotFound(new
        {
            success = false,
            message = "Room not found."
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

    var requestPasswordHash = HashRoomPassword(request.RoomPassword);

    if (session.RoomPasswordHash != requestPasswordHash)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "Invalid room password."
        });
    }

    var joinedMemberCount = session.Members.Count(x => x.Status == "Joined");

    var joinedSameAccount = session.Members.FirstOrDefault(x =>
        x.AccountId == request.AccountId &&
        x.Status == "Joined"
    );

    if (joinedSameAccount != null)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "Account is already joined in this session."
        });
    }

    var joinedSameCharacter = session.Members.FirstOrDefault(x =>
        x.CharacterId == request.CharacterId &&
        x.Status == "Joined"
    );

    if (joinedSameCharacter != null)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "Character is already joined in this session."
        });
    }

    if (joinedMemberCount >= session.MaxPlayers)
    {
        return Results.BadRequest(new
        {
            success = false,
            message = "Session is full."
        });
    }

    var joinToken = $"local-token-{Guid.NewGuid():N}";

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
        RoomName: session.RoomName,
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
    session.ServerRuntimeStatus = "Running";

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

    session.ServerRuntimeStatus = "Running";
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
 * 세션 멤버 퇴장 보고 API
 *
 * Dedicated Server가 특정 플레이어의 Logout을 감지했을 때 호출한다.
 *
 * 처리 흐름:
 * - 해당 멤버만 Left 처리
 * - 남은 Joined 멤버가 있으면 세션 유지
 * - 나간 멤버가 현재 파티장이면 다음 Joined 멤버에게 파티장 승계
 * - 남은 Joined 멤버가 없으면 세션 종료 처리
 */
app.MapPost("/api/sessions/member-left", async (
    SessionMemberLeftRequest request,
    DGDbContext db
) =>
{
    if (string.IsNullOrWhiteSpace(request.SessionId))
    {
        return Results.BadRequest(new SessionMemberLeftResponse(
            Success: false,
            SessionId: "",
            Status: "",
            CurrentLeaderAccountId: 0,
            RemainingPlayers: 0,
            ShouldShutdownServer: false,
            Message: "SessionId is required."
        ));
    }

    if (request.AccountId <= 0)
    {
        return Results.BadRequest(new SessionMemberLeftResponse(
            Success: false,
            SessionId: request.SessionId,
            Status: "",
            CurrentLeaderAccountId: 0,
            RemainingPlayers: 0,
            ShouldShutdownServer: false,
            Message: "AccountId must be greater than 0."
        ));
    }

    if (request.CharacterId <= 0)
    {
        return Results.BadRequest(new SessionMemberLeftResponse(
            Success: false,
            SessionId: request.SessionId,
            Status: "",
            CurrentLeaderAccountId: 0,
            RemainingPlayers: 0,
            ShouldShutdownServer: false,
            Message: "CharacterId must be greater than 0."
        ));
    }

    var session = await db.Sessions
        .Include(x => x.Members)
        .FirstOrDefaultAsync(x => x.SessionId == request.SessionId);

    if (session == null)
    {
        return Results.NotFound(new SessionMemberLeftResponse(
            Success: false,
            SessionId: request.SessionId,
            Status: "",
            CurrentLeaderAccountId: 0,
            RemainingPlayers: 0,
            ShouldShutdownServer: false,
            Message: "Session not found."
        ));
    }

    if (session.Status == "Ended")
    {
        return Results.Ok(new SessionMemberLeftResponse(
            Success: true,
            SessionId: session.SessionId,
            Status: session.Status,
            CurrentLeaderAccountId: session.CurrentLeaderAccountId,
            RemainingPlayers: 0,
            ShouldShutdownServer: true,
            Message: "Session is already ended."
        ));
    }

    var now = DateTime.UtcNow;

    var leavingMember = session.Members.FirstOrDefault(x =>
        x.AccountId == request.AccountId &&
        x.CharacterId == request.CharacterId &&
        x.Status == "Joined"
    );

    if (leavingMember == null)
    {
        var currentJoinedCount = session.Members.Count(x => x.Status == "Joined");

        return Results.BadRequest(new SessionMemberLeftResponse(
            Success: false,
            SessionId: session.SessionId,
            Status: session.Status,
            CurrentLeaderAccountId: session.CurrentLeaderAccountId,
            RemainingPlayers: currentJoinedCount,
            ShouldShutdownServer: currentJoinedCount <= 0,
            Message: "Joined member not found."
        ));
    }

    leavingMember.Status = "Left";

    if (leavingMember.LeftAtUtc == null)
    {
        leavingMember.LeftAtUtc = now;
    }

    var remainingMembers = session.Members
        .Where(x => x.Status == "Joined")
        .OrderBy(x => x.JoinedAtUtc)
        .ToList();

    if (remainingMembers.Count <= 0)
    {
        EndSession(session, now);

        await db.SaveChangesAsync();

        return Results.Ok(new SessionMemberLeftResponse(
            Success: true,
            SessionId: session.SessionId,
            Status: session.Status,
            CurrentLeaderAccountId: session.CurrentLeaderAccountId,
            RemainingPlayers: 0,
            ShouldShutdownServer: true,
            Message: "Last member left. Session ended."
        ));
    }

    if (session.CurrentLeaderAccountId == request.AccountId)
    {
        session.CurrentLeaderAccountId = remainingMembers[0].AccountId;
    }

    session.LastHeartbeatAtUtc = now;

    await db.SaveChangesAsync();

    return Results.Ok(new SessionMemberLeftResponse(
        Success: true,
        SessionId: session.SessionId,
        Status: session.Status,
        CurrentLeaderAccountId: session.CurrentLeaderAccountId,
        RemainingPlayers: remainingMembers.Count,
        ShouldShutdownServer: false,
        Message: "Member left."
    ));
});

/**
 * 세션 종료 보고 API
 *
 * Dedicated Server가 종료되거나,
 * 마지막 플레이어가 나갔을 때 호출한다.
 *
 * 처리 흐름:
 * - sessions.status를 Ended로 변경
 * - sessions.ended_at_utc 기록
 * - sessions.last_heartbeat_at_utc 갱신
 * - sessions.server_runtime_status를 Exited로 변경
 * - sessions.room_name / room_password_hash 비움
 * - 해당 세션의 Joined 멤버들을 Left 처리
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
        .Include(x => x.Members)
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

    EndSession(session, now);

    await db.SaveChangesAsync();

    return Results.Ok(new SessionEndedResponse(
        Success: true,
        SessionId: session.SessionId,
        Status: session.Status,
        EndedAtUtc: session.EndedAtUtc,
        Message: "Session ended."
    ));
});

static string NormalizeRoomName(string? roomName)
{
    return string.IsNullOrWhiteSpace(roomName)
        ? ""
        : roomName.Trim();
}

static string HashRoomPassword(string roomPassword)
{
    var bytes = Encoding.UTF8.GetBytes(roomPassword);
    var hashBytes = SHA256.HashData(bytes);

    return Convert.ToHexString(hashBytes);
}

static void EndSession(GameSession session, DateTime now)
{
    session.Status = "Ended";

    if (session.EndedAtUtc == null)
    {
        session.EndedAtUtc = now;
    }

    session.LastHeartbeatAtUtc = now;
    session.ServerRuntimeStatus = "Exited";
    session.RoomName = "";
    session.RoomPasswordHash = "";

    foreach (var member in session.Members)
    {
        if (member.Status == "Joined")
        {
            member.Status = "Left";

            if (member.LeftAtUtc == null)
            {
                member.LeftAtUtc = now;
            }
        }
    }
}

app.Run();

public record CreateSessionRequest(
    long AccountId,
    long CharacterId,
    string RegionId,
    string RoomName,
    string RoomPassword
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
    string RoomName,
    string RoomPassword,
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
    string RoomName,
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

public record SessionMemberLeftRequest(
    string SessionId,
    long AccountId,
    long CharacterId
);

public record SessionMemberLeftResponse(
    bool Success,
    string SessionId,
    string Status,
    long CurrentLeaderAccountId,
    int RemainingPlayers,
    bool ShouldShutdownServer,
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