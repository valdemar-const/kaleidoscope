param(
    [string]$File      = "",
    [string]$Grammar   = "example",
    [string]$StartRule = "main"
)

$AntlrJar = "$env:SCOOP\apps\antlr\current\antlr-4.13.2-complete.jar"

if (-not $File) {
    Write-Host "Usage: antlr-check <file.lang> [grammar] [start_rule]" -ForegroundColor Yellow
    Write-Host "Example: antlr-check listing.lang" -ForegroundColor Green
    exit 1
}

if (-not (Test-Path $File)) {
    Write-Host "File not found: $File" -ForegroundColor Red
    exit 1
}

# Правильный вызов: класс идет ДО -jar
java -cp $AntlrJar org.antlr.v4.gui.TestRig $Grammar $StartRule $File 2>&1 | Out-Null

if ($LASTEXITCODE -eq 0) {
    Write-Host "[ok]" -NoNewline -ForegroundColor Green
    Write-Host " $File"
    exit 0
} else {
    Write-Host "[no]" -NoNewline -ForegroundColor Red
    Write-Host " $File"
    exit 1
}