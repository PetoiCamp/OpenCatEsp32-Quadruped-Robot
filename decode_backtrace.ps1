# ESP32 Backtrace Decoder Script
# Usage: .\decode_backtrace.ps1 -ElfPath "path\to\file.elf" -Addresses @("0x400841b1", "0x40094461", ...)

param(
    [Parameter(Mandatory=$true, Position=0)]
    [string]$ElfPath,
    
    [Parameter(Mandatory=$true, Position=1)]
    [string[]]$Addresses
)

# Find xtensa-esp32-elf-addr2line.exe
$toolchainPaths = @(
    "$env:LOCALAPPDATA\Arduino15\packages\esp32\tools\xtensa-esp32-elf-gcc\*\bin\xtensa-esp32-elf-addr2line.exe",
    "$env:USERPROFILE\AppData\Local\Arduino15\packages\esp32\tools\xtensa-esp32-elf-gcc\*\bin\xtensa-esp32-elf-addr2line.exe"
)

$addr2line = $null
foreach ($path in $toolchainPaths) {
    $found = Get-ChildItem -Path $path -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        $addr2line = $found.FullName
        break
    }
}

if (-not $addr2line) {
    Write-Host "Error: Could not find xtensa-esp32-elf-addr2line.exe" -ForegroundColor Red
    Write-Host "Please ensure ESP32 Arduino toolchain is installed." -ForegroundColor Yellow
    exit 1
}

Write-Host "Using addr2line: $addr2line" -ForegroundColor Green
Write-Host "ELF file: $ElfPath" -ForegroundColor Green
Write-Host "`nDecoding addresses:" -ForegroundColor Cyan
Write-Host ("=" * 80)

# Decode each address
foreach ($addr in $Addresses) {
    Write-Host "`nAddress: $addr" -ForegroundColor Yellow
    $result = & $addr2line -pfiaC -e $ElfPath $addr 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host $result
    } else {
        Write-Host "Failed to decode: $result" -ForegroundColor Red
    }
}

Write-Host ("`n" + "=" * 80)
