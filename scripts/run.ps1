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

$BuildType = $BuildType.Trim()
$validBuildTypes = @("Debug", "Release", "RelWithDebInfo", "MinSizeRel")
if ($validBuildTypes -notcontains $BuildType) {
    Write-Error "Invalid BUILD_TYPE '$BuildType'. Allowed: $($validBuildTypes -join ", ")."
    exit 1
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

$cachePath = Join-Path $BuildDir "CMakeCache.txt"
$resetBuildType = $false
if (Test-Path $cachePath) {
    $cachedLine = Select-String -Path $cachePath -Pattern "^CMAKE_BUILD_TYPE:STRING=" -ErrorAction SilentlyContinue
    if ($cachedLine) {
        $cachedValue = $cachedLine.Line.Split("=", 2)[1].Trim()
        if ($validBuildTypes -notcontains $cachedValue) {
            $resetBuildType = $true
        }
    }
}

$cmakeArgs = @("-S", $RootDir, "-B", $BuildDir, "-DCMAKE_BUILD_TYPE:STRING=$BuildType")
if ($resetBuildType) {
    $cmakeArgs += @("-U", "CMAKE_BUILD_TYPE")
}

cmake @cmakeArgs
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
