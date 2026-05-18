using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace DGBackendApi.Migrations
{
    /// <inheritdoc />
    public partial class AddSessionHeartbeat : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<DateTime>(
                name: "last_heartbeat_at_utc",
                table: "sessions",
                type: "timestamp with time zone",
                nullable: true);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropColumn(
                name: "last_heartbeat_at_utc",
                table: "sessions");
        }
    }
}
