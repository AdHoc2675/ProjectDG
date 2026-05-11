using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace DGBackendApi.Migrations
{
    /// <inheritdoc />
    public partial class AddJoinTokenToSessionMembers : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<string>(
                name: "join_token",
                table: "session_members",
                type: "character varying(200)",
                maxLength: 200,
                nullable: false,
                defaultValue: "");

            migrationBuilder.CreateIndex(
                name: "IX_session_members_join_token",
                table: "session_members",
                column: "join_token");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropIndex(
                name: "IX_session_members_join_token",
                table: "session_members");

            migrationBuilder.DropColumn(
                name: "join_token",
                table: "session_members");
        }
    }
}
