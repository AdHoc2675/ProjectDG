using System;
using Microsoft.EntityFrameworkCore.Migrations;
using Npgsql.EntityFrameworkCore.PostgreSQL.Metadata;

#nullable disable

namespace DGBackendApi.Migrations
{
    /// <inheritdoc />
    public partial class InitialSessionSchema : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.CreateTable(
                name: "sessions",
                columns: table => new
                {
                    session_id = table.Column<string>(type: "character varying(100)", maxLength: 100, nullable: false),
                    owner_account_id = table.Column<long>(type: "bigint", nullable: false),
                    current_leader_account_id = table.Column<long>(type: "bigint", nullable: false),
                    region_id = table.Column<string>(type: "character varying(100)", maxLength: 100, nullable: false),
                    map_path = table.Column<string>(type: "character varying(300)", maxLength: 300, nullable: false),
                    server_ip = table.Column<string>(type: "character varying(100)", maxLength: 100, nullable: false),
                    server_port = table.Column<int>(type: "integer", nullable: false),
                    status = table.Column<string>(type: "character varying(30)", maxLength: 30, nullable: false),
                    max_players = table.Column<int>(type: "integer", nullable: false),
                    created_at_utc = table.Column<DateTime>(type: "timestamp with time zone", nullable: false),
                    started_at_utc = table.Column<DateTime>(type: "timestamp with time zone", nullable: true),
                    ended_at_utc = table.Column<DateTime>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_sessions", x => x.session_id);
                });

            migrationBuilder.CreateTable(
                name: "session_members",
                columns: table => new
                {
                    id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    session_id = table.Column<string>(type: "character varying(100)", maxLength: 100, nullable: false),
                    account_id = table.Column<long>(type: "bigint", nullable: false),
                    character_id = table.Column<long>(type: "bigint", nullable: false),
                    role = table.Column<string>(type: "character varying(30)", maxLength: 30, nullable: false),
                    status = table.Column<string>(type: "character varying(30)", maxLength: 30, nullable: false),
                    joined_at_utc = table.Column<DateTime>(type: "timestamp with time zone", nullable: false),
                    left_at_utc = table.Column<DateTime>(type: "timestamp with time zone", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_session_members", x => x.id);
                    table.ForeignKey(
                        name: "FK_session_members_sessions_session_id",
                        column: x => x.session_id,
                        principalTable: "sessions",
                        principalColumn: "session_id",
                        onDelete: ReferentialAction.Cascade);
                });

            migrationBuilder.CreateIndex(
                name: "IX_session_members_character_id",
                table: "session_members",
                column: "character_id");

            migrationBuilder.CreateIndex(
                name: "IX_session_members_session_id",
                table: "session_members",
                column: "session_id");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "session_members");

            migrationBuilder.DropTable(
                name: "sessions");
        }
    }
}
