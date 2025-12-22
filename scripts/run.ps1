param(
    [string]$BuildDir = $env:BUILD_DIR,
    [string]$BuildType = $env:BUILD_TYPE
)

if (-not $BuildDir) {
    $BuildDir = "build"
}

if (-not $BuildType) {
    $BuildType = "Debug"
}

$RootDir = Resolve-Path (Join-Path $PSScriptRoot "..")

cmake -S $RootDir -B $BuildDir -DCMAKE_BUILD_TYPE=$BuildType
cmake --build $BuildDir

$exeCandidates = @(
    Join-Path $BuildDir "AirTrace.exe",
    Join-Path $BuildDir $BuildType | Join-Path -ChildPath "AirTrace.exe",
    Join-Path $BuildDir "AirTrace",
    Join-Path (Join-Path $BuildDir $BuildType) "AirTrace"
)

$exePath = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $exePath) {
    Write-Error "AirTrace executable not found in $BuildDir"
    exit 1
}

& $exePath
