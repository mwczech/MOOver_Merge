# envSetup.ps1 - Pure PowerShell version
param()

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$TargetDir = Join-Path $ScriptDir "Melkens_Lib"
$SourceDir = Join-Path (Split-Path -Parent $ScriptDir) "Melkens_Lib"

Write-Host "Script directory: $ScriptDir"
Write-Host "Target directory: $TargetDir"
Write-Host "Source directory: $SourceDir"

# Check if target already exists
if (Test-Path $TargetDir) {
    Write-Host "Link/folder 'Melkens_Lib' already exists. Skipping creation." -ForegroundColor Yellow
    exit 0
}

# Check if source directory exists
if (-not (Test-Path $SourceDir)) {
    Write-Host "Error: Source directory '$SourceDir' does not exist!" -ForegroundColor Red
    exit 1
}

# Create junction
try {
    Write-Host "Creating junction: $TargetDir -> $SourceDir"
    New-Item -ItemType Junction -Path $TargetDir -Target $SourceDir | Out-Null
    Write-Host "Successfully created junction!" -ForegroundColor Green
} catch {
    Write-Host "Failed to create junction: $_" -ForegroundColor Red
    exit 1
}

Write-Host "Link creation completed successfully!" -ForegroundColor Green
