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
