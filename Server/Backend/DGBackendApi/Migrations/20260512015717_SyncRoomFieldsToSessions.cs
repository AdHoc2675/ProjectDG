using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace DGBackendApi.Migrations
{
    /// <inheritdoc />
    public partial class SyncRoomFieldsToSessions : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<string>(
                name: "room_name",
                table: "sessions",
                type: "character varying(100)",
                maxLength: 100,
                nullable: false,
                defaultValue: "");

            migrationBuilder.AddColumn<string>(
                name: "room_password_hash",
                table: "sessions",
                type: "character varying(128)",
                maxLength: 128,
                nullable: false,
                defaultValue: "");

            migrationBuilder.CreateIndex(
                name: "IX_sessions_room_name",
                table: "sessions",
                column: "room_name");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropIndex(
                name: "IX_sessions_room_name",
                table: "sessions");

            migrationBuilder.DropColumn(
                name: "room_name",
                table: "sessions");

            migrationBuilder.DropColumn(
                name: "room_password_hash",
                table: "sessions");
        }
    }
}
