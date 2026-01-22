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

$BuildType = $BuildType.Trim()
$validBuildTypes = @("Debug", "Release", "RelWithDebInfo", "MinSizeRel")
if ($validBuildTypes -notcontains $BuildType) {
    Write-Error "Invalid BUILD_TYPE '$BuildType'. Allowed: $($validBuildTypes -join ", ")."
    exit 1
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
