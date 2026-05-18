using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace DGBackendApi.Migrations
{
    /// <inheritdoc />
    public partial class AddDedicatedServerRuntimeFields : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<int>(
                name: "ServerProcessId",
                table: "sessions",
                type: "integer",
                nullable: true);

            migrationBuilder.AddColumn<string>(
                name: "ServerRuntimeStatus",
                table: "sessions",
                type: "text",
                nullable: false,
                defaultValue: "");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropColumn(
                name: "ServerProcessId",
                table: "sessions");

            migrationBuilder.DropColumn(
                name: "ServerRuntimeStatus",
                table: "sessions");
        }
    }
}
