using DGBackendApi.Entities;
using Microsoft.EntityFrameworkCore;

namespace DGBackendApi.Data;

public class DGDbContext : DbContext
{
    public DGDbContext(DbContextOptions<DGDbContext> options)
        : base(options)
    {
    }

    public DbSet<GameSession> Sessions => Set<GameSession>();
    public DbSet<SessionMember> SessionMembers => Set<SessionMember>();
    public DbSet<Account> Accounts => Set<Account>();
    public DbSet<GameCharacter> GameCharacters => Set<GameCharacter>();

    protected override void OnModelCreating(ModelBuilder modelBuilder)
    {
        base.OnModelCreating(modelBuilder);

        modelBuilder.Entity<Account>(entity =>
        {
            entity.ToTable("accounts");

            entity.HasKey(x => x.AccountId);

            entity.Property(x => x.AccountId)
                .HasColumnName("account_id");

            entity.Property(x => x.LoginId)
                .HasColumnName("login_id")
                .HasMaxLength(100);

            entity.Property(x => x.PasswordHash)
                .HasColumnName("password_hash")
                .HasMaxLength(500);

            entity.Property(x => x.DisplayName)
                .HasColumnName("display_name")
                .HasMaxLength(100);

            entity.Property(x => x.Status)
                .HasColumnName("status")
                .HasMaxLength(30);

            entity.Property(x => x.CreatedAtUtc)
                .HasColumnName("created_at_utc");

            entity.Property(x => x.LastLoginAtUtc)
                .HasColumnName("last_login_at_utc");

            entity.HasIndex(x => x.LoginId)
                .IsUnique();

            entity.HasMany(x => x.Characters)
                .WithOne(x => x.Account)
                .HasForeignKey(x => x.AccountId)
                .OnDelete(DeleteBehavior.Cascade);
        });

        modelBuilder.Entity<GameCharacter>(entity =>
        {
            entity.ToTable("game_characters");

            entity.HasKey(x => x.CharacterId);

            entity.Property(x => x.CharacterId)
                .HasColumnName("character_id");

            entity.Property(x => x.AccountId)
                .HasColumnName("account_id");

            entity.Property(x => x.CharacterName)
                .HasColumnName("character_name")
                .HasMaxLength(100);

            entity.Property(x => x.ClassTag)
                .HasColumnName("class_tag")
                .HasMaxLength(100);

            entity.Property(x => x.Level)
                .HasColumnName("level");

            entity.Property(x => x.Status)
                .HasColumnName("status")
                .HasMaxLength(30);

            entity.Property(x => x.CreatedAtUtc)
                .HasColumnName("created_at_utc");

            entity.Property(x => x.LastPlayedAtUtc)
                .HasColumnName("last_played_at_utc");

            entity.HasIndex(x => x.AccountId);
        });

        modelBuilder.Entity<GameSession>(entity =>
        {
            entity.ToTable("sessions");

            entity.HasKey(x => x.SessionId);

            entity.Property(x => x.SessionId)
                .HasColumnName("session_id")
                .HasMaxLength(100);

            entity.Property(x => x.OwnerAccountId)
                .HasColumnName("owner_account_id");

            entity.Property(x => x.CurrentLeaderAccountId)
                .HasColumnName("current_leader_account_id");

            entity.Property(x => x.RegionId)
                .HasColumnName("region_id")
                .HasMaxLength(100);

            entity.Property(x => x.RoomName)
                .HasColumnName("room_name")
                .HasMaxLength(100);

            entity.Property(x => x.RoomPasswordHash)
                .HasColumnName("room_password_hash")
                .HasMaxLength(128);

            entity.Property(x => x.MapPath)
                .HasColumnName("map_path")
                .HasMaxLength(300);

            entity.Property(x => x.ServerIp)
                .HasColumnName("server_ip")
                .HasMaxLength(100);

            entity.Property(x => x.ServerPort)
                .HasColumnName("server_port");

            entity.Property(x => x.ServerProcessId)
                .HasColumnName("server_process_id");

            entity.Property(x => x.ServerRuntimeStatus)
                .HasColumnName("server_runtime_status")
                .HasMaxLength(30);

            entity.Property(x => x.Status)
                .HasColumnName("status")
                .HasMaxLength(30);

            entity.Property(x => x.MaxPlayers)
                .HasColumnName("max_players");

            entity.Property(x => x.CreatedAtUtc)
                .HasColumnName("created_at_utc");

            entity.Property(x => x.StartedAtUtc)
                .HasColumnName("started_at_utc");

            entity.Property(x => x.LastHeartbeatAtUtc)
                .HasColumnName("last_heartbeat_at_utc");

            entity.Property(x => x.EndedAtUtc)
                .HasColumnName("ended_at_utc");

            entity.HasMany(x => x.Members)
                .WithOne(x => x.Session)
                .HasForeignKey(x => x.SessionId)
                .OnDelete(DeleteBehavior.Cascade);

            entity.HasIndex(x => x.RoomName);
        });

        modelBuilder.Entity<SessionMember>(entity =>
        {
            entity.ToTable("session_members");

            entity.HasKey(x => x.Id);

            entity.Property(x => x.Id)
                .HasColumnName("id");

            entity.Property(x => x.SessionId)
                .HasColumnName("session_id")
                .HasMaxLength(100);

            entity.Property(x => x.AccountId)
                .HasColumnName("account_id");

            entity.Property(x => x.CharacterId)
                .HasColumnName("character_id");

            entity.Property(x => x.Role)
                .HasColumnName("role")
                .HasMaxLength(30);

            entity.Property(x => x.Status)
                .HasColumnName("status")
                .HasMaxLength(30);

            entity.Property(x => x.JoinToken)
                .HasColumnName("join_token")
                .HasMaxLength(200)
                .HasDefaultValue("");

            entity.Property(x => x.JoinedAtUtc)
                .HasColumnName("joined_at_utc");

            entity.Property(x => x.LeftAtUtc)
                .HasColumnName("left_at_utc");

            entity.HasIndex(x => x.SessionId);

            entity.HasIndex(x => x.CharacterId);

            entity.HasIndex(x => x.JoinToken);
        });
    }
}