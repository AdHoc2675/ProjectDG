using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace DGBackendApi.Migrations
{
    /// <inheritdoc />
    public partial class RenameDedicatedServerRuntimeColumns : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.RenameColumn(
                name: "ServerRuntimeStatus",
                table: "sessions",
                newName: "server_runtime_status");

            migrationBuilder.RenameColumn(
                name: "ServerProcessId",
                table: "sessions",
                newName: "server_process_id");

            migrationBuilder.AlterColumn<string>(
                name: "server_runtime_status",
                table: "sessions",
                type: "character varying(30)",
                maxLength: 30,
                nullable: false,
                oldClrType: typeof(string),
                oldType: "text");
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.RenameColumn(
                name: "server_runtime_status",
                table: "sessions",
                newName: "ServerRuntimeStatus");

            migrationBuilder.RenameColumn(
                name: "server_process_id",
                table: "sessions",
                newName: "ServerProcessId");

            migrationBuilder.AlterColumn<string>(
                name: "ServerRuntimeStatus",
                table: "sessions",
                type: "text",
                nullable: false,
                oldClrType: typeof(string),
                oldType: "character varying(30)",
                oldMaxLength: 30);
        }
    }
}
