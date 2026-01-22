param(
    [string]$BuildDir = $env:BUILD_DIR,
    [string]$BuildType = $env:BUILD_TYPE
)

if (-not $BuildDir) {
    $BuildDir = ""
}

if (-not $BuildType) {
    $BuildType = "Debug"
}

$RootDir = Resolve-Path (Join-Path $PSScriptRoot "..")

if (-not $env:CMAKE_GENERATOR) {
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    if ($ninja) {
        $env:CMAKE_GENERATOR = "Ninja"
    }
}

if (-not $BuildDir) {
    if ($env:CMAKE_GENERATOR -eq "Ninja") {
        $BuildDir = "build\\tests-ninja"
    } else {
        $BuildDir = "build\\tests"
    }
}

if (-not [System.IO.Path]::IsPathRooted($BuildDir)) {
    $BuildDir = Join-Path $RootDir $BuildDir
}

$cmakeArgs = @("-S", $RootDir, "-B", $BuildDir, "-DCMAKE_BUILD_TYPE=$BuildType")
if ($env:CMAKE_GENERATOR) {
    $cmakeArgs += @("-G", $env:CMAKE_GENERATOR)
}

cmake @cmakeArgs
cmake --build $BuildDir --target AirTraceCoreTests AirTraceEdgeCaseTests

Push-Location $BuildDir
try {
    ctest --output-on-failure
} finally {
    Pop-Location
}
