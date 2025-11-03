Write-Host "Testing grammar compliance..."
Write-Host ""

rm -r .check

Get-ChildItem "listings\*.ks" | ForEach-Object {
    antlr-check "listings\$($_.Name)" kaleidoscope chunk
}
