Write-Host "Testing grammar compliance..."
Write-Host ""

Get-ChildItem "listings\*.ks" | ForEach-Object {
    antlr-check "listings\$($_.Name)" kaleidoscope program
}
