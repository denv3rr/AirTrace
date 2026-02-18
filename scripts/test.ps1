param(
    [string]$BuildDir = $env:BUILD_DIR,
    [string]$BuildType = $env:BUILD_TYPE
)

function Assert-LastExitCode([string]$Context) {
    if ($LASTEXITCODE -ne 0) {
        Write-Error "$Context failed with exit code $LASTEXITCODE."
        exit $LASTEXITCODE
    }
}

if (-not $BuildDir) {
    $BuildDir = ""
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
if ($env:CMAKE_GENERATOR) {
    $cmakeArgs += @("-G", $env:CMAKE_GENERATOR)
}
if ($resetBuildType) {
    $cmakeArgs += @("-U", "CMAKE_BUILD_TYPE")
}

cmake @cmakeArgs
Assert-LastExitCode "CMake configure"
$moduleTargets = @("airtrace_core", "airtrace_adapters_contract", "airtrace_tools", "airtrace_ui", "airtrace_ui_harness")
$sdkEnabled = Select-String -Path $cachePath -Pattern "^AIRTRACE_BUILD_ADAPTER_SDK:BOOL=ON$" -Quiet -ErrorAction SilentlyContinue
if ($sdkEnabled) {
    $moduleTargets += "airtrace_adapters_sdk"
}
$testTargets = @("AirTraceCoreTests", "AirTraceEdgeCaseTests", "AirTraceUiTests", "AirTraceHarnessTests", "AirTraceIntegrationTests")
$buildTargets = $moduleTargets + $testTargets
cmake --build $BuildDir --target @buildTargets
Assert-LastExitCode "CMake build"

Push-Location $BuildDir
try {
    $tempOutput = [System.IO.Path]::GetTempFileName()
    try {
        cmd /c "ctest -Q --output-on-failure --output-log `"$tempOutput`" > NUL 2>&1"
        $ctestExit = $LASTEXITCODE
        Get-Content -Path $tempOutput | ForEach-Object {
            $_ -replace "Total Test time \(real\)", "Total Test time"
        }
        if ($ctestExit -ne 0) {
            exit $ctestExit
        }
    } finally {
        Remove-Item -Path $tempOutput -ErrorAction SilentlyContinue
    }
} finally {
    Pop-Location
}
