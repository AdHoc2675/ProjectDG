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

    protected override void OnModelCreating(ModelBuilder modelBuilder)
    {
        base.OnModelCreating(modelBuilder);

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

            entity.Property(x => x.MapPath)
                .HasColumnName("map_path")
                .HasMaxLength(300);

            entity.Property(x => x.ServerIp)
                .HasColumnName("server_ip")
                .HasMaxLength(100);

            entity.Property(x => x.ServerPort)
                .HasColumnName("server_port");

            entity.Property(x => x.Status)
                .HasColumnName("status")
                .HasMaxLength(30);

            entity.Property(x => x.MaxPlayers)
                .HasColumnName("max_players");

            entity.Property(x => x.CreatedAtUtc)
                .HasColumnName("created_at_utc");

            entity.Property(x => x.StartedAtUtc)
                .HasColumnName("started_at_utc");

            entity.Property(x => x.EndedAtUtc)
                .HasColumnName("ended_at_utc");

            entity.HasMany(x => x.Members)
                .WithOne(x => x.Session)
                .HasForeignKey(x => x.SessionId)
                .OnDelete(DeleteBehavior.Cascade);
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

            entity.Property(x => x.JoinedAtUtc)
                .HasColumnName("joined_at_utc");

            entity.Property(x => x.LeftAtUtc)
                .HasColumnName("left_at_utc");

            entity.HasIndex(x => x.SessionId);

            entity.HasIndex(x => x.CharacterId);
        });
    }
}