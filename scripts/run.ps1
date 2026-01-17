param(
    [string]$BuildDir = $env:BUILD_DIR,
    [string]$BuildType = $env:BUILD_TYPE,
    [string]$Target = $env:AIRTRACE_TARGET,
    [string]$ConfigPath = $env:AIRTRACE_CONFIG
)

if (-not $BuildDir) {
    $BuildDir = "build"
}

if (-not $BuildType) {
    $BuildType = "Debug"
}

if (-not $Target) {
    $Target = "AirTrace"
}

if (-not $ConfigPath) {
    $ConfigPath = "configs/sim_default.cfg"
}

$RootDir = Resolve-Path (Join-Path $PSScriptRoot "..")
if (-not [System.IO.Path]::IsPathRooted($BuildDir)) {
    $BuildDir = Join-Path $RootDir $BuildDir
}

cmake -S $RootDir -B $BuildDir -DCMAKE_BUILD_TYPE=$BuildType
cmake --build $BuildDir

$exeCandidates = @(
    Join-Path $BuildDir "$Target.exe",
    Join-Path $BuildDir $BuildType | Join-Path -ChildPath "$Target.exe",
    Join-Path $BuildDir "$Target",
    Join-Path (Join-Path $BuildDir $BuildType) "$Target"
)

$exePath = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $exePath) {
    Write-Error "$Target executable not found in $BuildDir"
    exit 1
}

Push-Location $RootDir
try {
    if ($Target -eq "AirTraceSimExample") {
        & $exePath $ConfigPath
    } else {
        & $exePath
    }
} finally {
    Pop-Location
}
