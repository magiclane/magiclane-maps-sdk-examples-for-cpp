# SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
# SPDX-License-Identifier: Apache-2.0
#
# Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#Requires -Version 5.1

<#
.SYNOPSIS
    Build all Maps SDK for C++ examples for Windows x86_64.

.DESCRIPTION
    Windows counterpart of build_all.sh. Extracts the Maps SDK for C++
    archive, checks build prerequisites, then configures and builds all
    examples using the CMake presets from CMakePresets.json.
    Supports MSVC or Clang/LLVM toolchain. Run with -Help for all options.

.NOTES
    Version: 1.0
#>

[CmdletBinding()]
param(
    [string]$SdkArchive = '',

    [ValidateSet('Debug', 'Release', 'RelWithDebInfo')]
    [string]$BuildType = 'Release',

    [ValidateSet('vs2022', 'vs2019', 'vs2019-toolset', 'ninja')]
    [string]$Generator = 'vs2022',

    [ValidateSet('MSVC', 'LLVM')]
    [string]$Toolchain = 'MSVC',

    [string]$Jobs = 'auto',

    [string]$ApiToken = '',

    [switch]$WithSdl,

    [ValidateSet('address')]
    [string]$WithSanitizer = '',

    [switch]$Clean,

    [ValidateSet('auto', 'plain', 'colored', 'verbose')]
    [string]$Console = 'auto',

    [switch]$Help
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

$Script:ProgName          = Split-Path -Leaf $PSCommandPath
$Script:ScriptDir         = $PSScriptRoot
$Script:GeneratorExplicit = $PSBoundParameters.ContainsKey('Generator')
$Script:UseColors         = $true
$Script:ShowExitMessage   = $true
$Script:CleanOnExit       = $false
$Script:ExitCode          = 0
$Script:JobCount          = 1

# =============================================================================
# Logging
# =============================================================================

function Get-LogTimestamp {
    Get-Date -Format 'yyyy-MM-dd HH:mm:ss'
}

function Write-LogLine {
    param(
        [string]$Label,
        [System.ConsoleColor]$Color,
        [string]$Message
    )

    $prefix = "[$(Get-LogTimestamp)] [$Label]"
    if ($Script:UseColors) {
        Write-Host $prefix -ForegroundColor $Color -NoNewline
        Write-Host " $Message"
    } else {
        Write-Host "$prefix $Message"
    }
}

function Write-LogInfo    { param([string]$Message) Write-LogLine -Label 'INFO'    -Color Cyan   -Message $Message }
function Write-LogSuccess { param([string]$Message) Write-LogLine -Label 'SUCCESS' -Color Green  -Message $Message }
function Write-LogWarning { param([string]$Message) Write-LogLine -Label 'WARNING' -Color Yellow -Message $Message }
function Write-LogError   { param([string]$Message) Write-LogLine -Label 'ERROR'   -Color Red    -Message $Message }

function Write-LogStep {
    param([string]$Message)

    Write-Host ''
    Write-LogLine -Label 'STEP' -Color Blue -Message $Message
    Write-Host ''
}

function Set-ConsoleMode {
    switch ($Console) {
        'auto' {
            if ([System.Console]::IsOutputRedirected) {
                $Script:UseColors = $false
            }
        }
        'plain' {
            $Script:UseColors = $false
        }
        'colored' {
            $Script:UseColors = $true
        }
        'verbose' {
            $Script:UseColors = $true
            Set-PSDebug -Trace 1
        }
    }
}

# =============================================================================
# Helpers
# =============================================================================

function Test-Command {
    param([string]$Name)

    return [bool](Get-Command -Name $Name -CommandType Application -ErrorAction SilentlyContinue)
}

function Test-IsWindows64 {
    $isWin = $true
    if ($PSVersionTable.PSVersion.Major -ge 6) {
        $isWin = $IsWindows
    }
    return ($isWin -and [System.Environment]::Is64BitOperatingSystem)
}

function Test-IsCI {
    if ($env:CI -and ($env:CI -ne 'false') -and ($env:CI -ne '0')) {
        return $true
    }

    $ciVars = @(
        'GITHUB_ACTIONS'
        'GITLAB_CI'
        'JENKINS_URL'
        'TEAMCITY_VERSION'
        'BUILDKITE'
        'CIRCLECI'
        'TRAVIS'
        'APPVEYOR'
        'TF_BUILD'
        'BITBUCKET_BUILD_NUMBER'
        'DRONE'
        'SEMAPHORE'
        'CODEBUILD_BUILD_ID'
    )

    foreach ($var in $ciVars) {
        if ([System.Environment]::GetEnvironmentVariable($var)) {
            return $true
        }
    }

    return $false
}

function Invoke-DistClean {
    if (-not $Script:CleanOnExit) {
        return
    }

    $buildDir = Join-Path $Script:ScriptDir 'BUILD'
    if (Test-Path -LiteralPath $buildDir) {
        Remove-Item -LiteralPath $buildDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

function Show-Usage {
    $Script:ShowExitMessage = $false

    $text = @"

Usage: $($Script:ProgName) [options]

Options:
    [OPTIONAL] -Help
                    Show this help message

    [REQUIRED] -SdkArchive <path>
                    Set path to the Maps SDK for C++ archive

    [OPTIONAL] -BuildType <Debug|Release|RelWithDebInfo>
                    Set build type. Default is 'Release'

    [OPTIONAL] -Generator <vs2022|vs2019|vs2019-toolset|ninja>
                    Set CMake preset family. Default is 'vs2022'

                    vs2019 requires a full VS2019 installation.
                    vs2019-toolset uses the v142 toolset via VS2022
                    (requires VS2022 + v142 build tools).
                    ninja imports the MSVC environment automatically via
                    vcvarsall.bat when not already present

    [OPTIONAL] -Toolchain <MSVC|LLVM>
                    Set compiler toolchain. Default is 'MSVC'

                    LLVM builds the examples with clang-cl and lld-link
                    and always uses the Ninja presets, matching
                    build_windows_sdk.ps1. Use it when your SDK archive
                    was built with -Toolchain LLVM

                    NOTE: -WithSanitizer is not supported with the
                    LLVM toolchain

    [OPTIONAL] -Jobs <auto|int>
                    Set parallel build jobs. Default is 'auto' (number
                    of CPUs). With the Visual Studio generators this
                    builds multiple example projects concurrently, so
                    executables also link in parallel

    [OPTIONAL] -ApiToken <token>
                    Specify API token to be hardcoded into examples.
                    Can also be set via GEM_TOKEN environment variable
                    (command line takes precedence)

    [OPTIONAL] -WithSdl
                    Prefer using SDL for OpenGL context creation
                    instead of GLFW

    [OPTIONAL] -WithSanitizer <address>
                    Configure for Windows 64-bit RelWithDebInfo build
                    (+GLFW, VS2022) with specified sanitizer enabled

                    NOTE: Sanitizer builds use the GLFW backend and the
                    VS2022 generator, even if -WithSdl or another
                    -Generator is specified

    [OPTIONAL] -Clean
                    Remove build artifacts on exit.
                    Default is on in CI, off locally

    [OPTIONAL] -Console <auto|plain|colored|verbose>
                    Specifies which type of console output to generate.
                    Default is 'auto'

                    auto:    colored when attached to a terminal,
                             plain otherwise
                    plain:   plain text only; disables all color
                    colored: colored output
                    verbose: colored output and verbose logging

"@

    Write-Host $text
}

# =============================================================================
# Prerequisites
# =============================================================================

function Get-VsWherePath {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        return $vswhere
    }
    return $null
}

function Get-VsInstallationPath {
    param([string]$VersionRange = '[16.0,18.0)')

    $vswhere = Get-VsWherePath
    if (-not $vswhere) {
        return $null
    }

    $installPath = & $vswhere -products * -version $VersionRange `
        -requires 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64' `
        -property installationPath -latest

    if ($installPath) {
        return ([string]$installPath).Trim()
    }
    return $null
}

function Import-MsvcEnvironment {
    # Already running in a Developer PowerShell / vcvars environment
    if (Test-Command 'cl') {
        Write-LogInfo "Found cl: $((Get-Command cl -CommandType Application | Select-Object -First 1).Source)"
        return
    }

    # Run vcvarsall.bat in a cmd subshell, dump the environment and import the diff.
    Write-LogInfo 'MSVC environment not detected. Importing from vcvarsall.bat...'

    $installPath = Get-VsInstallationPath
    if (-not $installPath) {
        Write-LogError 'Visual Studio with C++ build tools not found; cannot set up the MSVC environment.'
        Write-LogError 'Install Visual Studio 2019/2022, or run this script from a "Developer PowerShell for VS" prompt.'
        exit 1
    }

    $vcvarsall = Join-Path $installPath 'VC\Auxiliary\Build\vcvarsall.bat'
    if (-not (Test-Path -LiteralPath $vcvarsall)) {
        Write-LogError "vcvarsall.bat not found at: $vcvarsall"
        exit 1
    }

    $envBefore = @{}
    Get-ChildItem env: | ForEach-Object { $envBefore[$_.Name] = $_.Value }

    $tempFile = [System.IO.Path]::GetTempFileName()
    $batFile = [System.IO.Path]::GetTempFileName() + '.bat'
    Set-Content -Path $batFile -Value ('@call "{0}" x64 && set > "{1}"' -f $vcvarsall, $tempFile) -Encoding ASCII

    $prevEAP = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    cmd /c $batFile 2>$null
    $vcvarsExitCode = $LASTEXITCODE
    $ErrorActionPreference = $prevEAP

    Remove-Item -LiteralPath $batFile -Force -ErrorAction SilentlyContinue

    if ($vcvarsExitCode -ne 0) {
        Remove-Item -LiteralPath $tempFile -Force -ErrorAction SilentlyContinue
        Write-LogError 'Failed to run vcvarsall.bat x64'
        exit 1
    }

    Get-Content -LiteralPath $tempFile | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            $name = $Matches[1]
            $value = $Matches[2]
            if (-not $envBefore.ContainsKey($name) -or ($envBefore[$name] -ne $value)) {
                [System.Environment]::SetEnvironmentVariable($name, $value, 'Process')
            }
        }
    }
    Remove-Item -LiteralPath $tempFile -Force -ErrorAction SilentlyContinue

    if (-not (Test-Command 'cl')) {
        Write-LogError 'cl.exe still not found after importing the MSVC environment.'
        exit 1
    }
    Write-LogSuccess "MSVC x64 environment imported ($((Get-Command cl -CommandType Application | Select-Object -First 1).Source))"
}

function Test-VisualStudioPrerequisites {
    if (-not (Get-VsWherePath)) {
        Write-LogWarning 'vswhere.exe not found; unable to verify the Visual Studio installation.'
        Write-LogWarning 'Continuing anyway - CMake will fail if Visual Studio is missing.'
        return
    }

    $versionRange = ''
    $displayName = ''

    switch ($Generator) {
        'vs2019' {
            $versionRange = '[16.0,17.0)'
            $displayName = 'Visual Studio 2019'
        }
        default {
            # vs2022 and vs2019-toolset both need a VS2022 installation
            $versionRange = '[17.0,18.0)'
            $displayName = 'Visual Studio 2022'
        }
    }

    $installPath = Get-VsInstallationPath -VersionRange $versionRange

    if (-not $installPath) {
        Write-LogError "$displayName with C++ build tools not found."
        Write-LogError 'Please install it from https://visualstudio.microsoft.com/downloads/'
        if ($Generator -eq 'vs2019') {
            Write-LogError 'Alternatively, use -Generator vs2019-toolset if you have VS2022 with the v142 build tools.'
        }
        exit 1
    }

    Write-LogInfo "Found ${displayName}: ${installPath}"

    if ($Generator -eq 'vs2019-toolset') {
        Write-LogInfo 'Using the v142 toolset via VS2022. Make sure the "MSVC v142" build tools component is installed.'
    }
}

function Test-NinjaPrerequisites {
    if (-not (Test-Command 'ninja')) {
        Write-LogError 'ninja command not found'
        Write-LogError 'Please install Ninja: https://ninja-build.org/'
        exit 1
    }
    Write-LogInfo "Found ninja: $(& ninja --version)"

    Import-MsvcEnvironment
}

function Test-LlvmPrerequisites {
    # LLVM toolchain builds always go through the Ninja presets
    if (-not (Test-Command 'ninja')) {
        Write-LogError 'ninja command not found'
        Write-LogError 'Please install Ninja: https://ninja-build.org/'
        exit 1
    }
    Write-LogInfo "Found ninja: $(& ninja --version)"

    # clang-cl and lld-link need INCLUDE/LIB from the MSVC environment
    Import-MsvcEnvironment

    if (-not (Test-Command 'clang-cl')) {
        # Fallback: try the LLVM toolset bundled with Visual Studio.
        $installPath = Get-VsInstallationPath
        if ($installPath) {
            $vsClangPath = Join-Path $installPath 'VC\Tools\Llvm\x64\bin'
            if (Test-Path -LiteralPath (Join-Path $vsClangPath 'clang-cl.exe')) {
                Write-LogInfo "Using the Visual Studio LLVM toolset: $vsClangPath"
                $env:PATH = "$vsClangPath;$env:PATH"
            }
        }
    }

    if (-not (Test-Command 'clang-cl')) {
        Write-LogError 'clang-cl.exe not found.'
        Write-LogError "Install LLVM/Clang via: VS Installer -> Individual Components -> 'C++ Clang Compiler for Windows',"
        Write-LogError 'or standalone from https://releases.llvm.org/'
        exit 1
    }
    Write-LogInfo "Found clang-cl: $((Get-Command clang-cl -CommandType Application | Select-Object -First 1).Source)"

    if (-not (Test-Command 'lld-link')) {
        Write-LogError 'lld-link.exe not found (it is installed together with clang-cl).'
        exit 1
    }
    Write-LogInfo "Found lld-link: $((Get-Command lld-link -CommandType Application | Select-Object -First 1).Source)"
}

# =============================================================================
# SDK extraction
# =============================================================================

function Expand-SdkArchive {
    Write-LogStep 'Extracting SDK archive...'

    $sdkDir = Join-Path $Script:ScriptDir 'SDK'

    if (-not (Test-Path -LiteralPath $sdkDir)) {
        New-Item -ItemType Directory -Path $sdkDir -Force | Out-Null
    }

    # Always start from a clean SDK directory, keeping the tracked .gitkeep
    Write-LogInfo 'Cleaning existing SDK directory...'
    Get-ChildItem -LiteralPath $sdkDir -Force |
        Where-Object { $_.Name -ne '.gitkeep' } |
        Remove-Item -Recurse -Force

    if (Test-Command 'tar') {
        # bsdtar (built into Windows 10 1803+) handles both .zip and .tar.gz
        & tar -xf $SdkArchive --strip-components=1 -C $sdkDir
        if ($LASTEXITCODE -ne 0) {
            Write-LogError 'SDK extraction failed'
            exit 1
        }
    } elseif ([System.IO.Path]::GetExtension($SdkArchive) -eq '.zip') {
        # Fallback: Expand-Archive + strip the single top-level directory
        $tempDir = Join-Path ([System.IO.Path]::GetTempPath()) ("maps-sdk-extract-" + [System.IO.Path]::GetRandomFileName())
        New-Item -ItemType Directory -Path $tempDir -Force | Out-Null
        try {
            Expand-Archive -LiteralPath $SdkArchive -DestinationPath $tempDir -Force

            $topLevel = @(Get-ChildItem -LiteralPath $tempDir -Force)
            if (($topLevel.Count -eq 1) -and $topLevel[0].PSIsContainer) {
                Get-ChildItem -LiteralPath $topLevel[0].FullName -Force |
                    Move-Item -Destination $sdkDir -Force
            } else {
                $topLevel | Move-Item -Destination $sdkDir -Force
            }
        } catch {
            Write-LogError "SDK extraction failed: $($_.Exception.Message)"
            exit 1
        } finally {
            Remove-Item -LiteralPath $tempDir -Recurse -Force -ErrorAction SilentlyContinue
        }
    } else {
        Write-LogError 'tar.exe not found and the archive is not a .zip file.'
        Write-LogError 'Please install tar (included with Windows 10 1803+) or provide a .zip archive.'
        exit 1
    }

    Write-LogSuccess 'SDK archive extracted successfully'
}

function Initialize-CMakeLists {
    if (Test-Path -LiteralPath (Join-Path $Script:ScriptDir 'CMakeLists.txt.SDK')) {
        Write-LogInfo 'Preparing CMakeLists.txt files from SDK templates...'
        Get-ChildItem -LiteralPath $Script:ScriptDir -Recurse -Filter 'CMakeLists.txt.SDK' -File |
            ForEach-Object {
                $destination = $_.FullName -replace '\.txt\.SDK$', '.txt'
                Copy-Item -LiteralPath $_.FullName -Destination $destination -Force
            }
    }
}

# =============================================================================
# Build
# =============================================================================

function Build-AllExamples {
    Write-LogStep 'Building all examples...'

    Push-Location $Script:ScriptDir
    try {
        Initialize-CMakeLists

        $openGlBackend = 'glfw'
        if ($WithSdl) {
            if ($WithSanitizer) {
                Write-LogWarning 'Sanitizer builds use GLFW backend. Ignoring -WithSdl flag.'
            } else {
                $openGlBackend = 'sdl'
            }
        }

        # Always clean build
        if (Test-Path -LiteralPath 'BUILD') {
            Write-LogInfo 'Removing existing BUILD directory...'
            Remove-Item -LiteralPath 'BUILD' -Recurse -Force
        }

        # LLVM toolchain builds always use the Ninja presets (the VS generator's
        # ClangCL toolset is avoided for MSBuild/include-path reasons).
        $effectiveGenerator = $Generator
        if (($Toolchain -eq 'LLVM') -and ($effectiveGenerator -ne 'ninja')) {
            if ($Script:GeneratorExplicit) {
                Write-LogWarning "LLVM toolchain builds use the Ninja generator. Ignoring -Generator $Generator."
            }
            $effectiveGenerator = 'ninja'
        }

        $buildTypeLower = $BuildType.ToLowerInvariant()
        $configPresetName = ''
        $buildPresetName = ''

        if ($WithSanitizer) {
            # Only the VS2022 + GLFW address sanitizer preset exists on Windows
            if ($Generator -ne 'vs2022') {
                Write-LogWarning "Sanitizer builds use the VS2022 generator. Ignoring -Generator $Generator."
            }
            $sanitizerLower = $WithSanitizer.ToLowerInvariant()
            $configPresetName = "windows-glfw-vs2022-$sanitizerLower-sanitizer"
            $buildPresetName = "windows-glfw-vs2022-$sanitizerLower-sanitizer-all"
        } elseif ($effectiveGenerator -eq 'ninja') {
            $configPresetName = "windows-$openGlBackend-ninja-$buildTypeLower"
            $buildPresetName = "windows-$openGlBackend-ninja-$buildTypeLower-all"
        } else {
            $configPresetName = "windows-$openGlBackend-$effectiveGenerator"
            $buildPresetName = "windows-$openGlBackend-$effectiveGenerator-$buildTypeLower-all"
        }

        Write-LogInfo "Configure preset: $configPresetName"
        Write-LogInfo "Build preset: $buildPresetName"

        $cmakeArgs = @('--preset', $configPresetName)

        if ($Toolchain -eq 'LLVM') {
            $cmakeArgs += '-DCMAKE_C_COMPILER=clang-cl'
            $cmakeArgs += '-DCMAKE_CXX_COMPILER=clang-cl'
            $cmakeArgs += '-DCMAKE_LINKER=lld-link'
        }

        $token = $ApiToken
        if (-not $token) {
            $token = $env:GEM_TOKEN
        }

        if ($token) {
            $cmakeArgs += "-DGEM_TOKEN=$token"
        } else {
            Write-LogWarning 'No token set. You can still test your apps, but a watermark will be displayed, and all the online services including mapping, searching, routing, etc. will slow down after a few minutes.'
        }

        Write-LogStep 'Running CMake configure...'
        & cmake @cmakeArgs
        if ($LASTEXITCODE -ne 0) {
            Write-LogError "CMake configure failed with exit code $LASTEXITCODE"
            exit 1
        }

        Write-LogStep 'Running CMake build...'
        # --parallel maps to msbuild /m:N for the VS generators (projects
        # build and link concurrently) and to ninja -jN for Ninja
        & cmake --build --preset $buildPresetName --parallel $Script:JobCount --verbose
        if ($LASTEXITCODE -ne 0) {
            Write-LogError "CMake build failed with exit code $LASTEXITCODE"
            exit 1
        }
    } finally {
        Pop-Location
    }

    Write-LogSuccess 'Build completed successfully'

    # Print sanitizer runtime instructions if applicable
    if ($WithSanitizer) {
        Show-SanitizerInstructions -ConfigPresetName $configPresetName
    }
}

function Show-SanitizerInstructions {
    param([string]$ConfigPresetName)

    Write-LogStep 'Sanitizer Runtime Instructions'

    Write-LogInfo "Your build was compiled with $WithSanitizer Sanitizer enabled."
    Write-LogInfo ''

    if ($WithSanitizer.ToLowerInvariant() -eq 'address') {
        Write-LogInfo 'AddressSanitizer (ASan) detects memory errors such as:'
        Write-LogInfo '  - Buffer overflows (stack, heap, global)'
        Write-LogInfo '  - Use-after-free, use-after-return'
        Write-LogInfo ''
        Write-LogInfo 'Your applications are located under:'
        Write-LogInfo "  BUILD\$ConfigPresetName\RelWithDebInfo\bin\<YourApp>.exe"
        Write-LogInfo ''
        Write-LogInfo 'If the application fails to start due to a missing'
        Write-LogInfo 'clang_rt.asan_dynamic-x86_64.dll, run it from a'
        Write-LogInfo '"Developer PowerShell for VS 2022" prompt, or copy the ASan runtime'
        Write-LogInfo 'DLLs from the Visual Studio installation next to the executable.'
    }

    Write-LogInfo ''
    Write-LogInfo 'For more information on sanitizer options, see:'
    Write-LogInfo '  https://github.com/google/sanitizers/wiki'
}

# =============================================================================
# Main
# =============================================================================

function Invoke-Main {
    Set-ConsoleMode

    if ($Help) {
        Show-Usage
        return
    }

    if (-not (Test-IsWindows64)) {
        Write-LogError 'This script only supports 64-bit Windows.'
        Write-LogError "Detected: $([System.Environment]::OSVersion.Platform), 64-bit OS: $([System.Environment]::Is64BitOperatingSystem)"
        exit 1
    }

    if (Test-IsCI) {
        $Script:CleanOnExit = $true
    }
    if ($Clean) {
        $Script:CleanOnExit = $true
    }

    Write-LogStep 'Checking prerequisites...'

    if (-not $SdkArchive -or -not (Test-Path -LiteralPath $SdkArchive -PathType Leaf)) {
        Write-LogError 'You must provide local path to SDK archive using -SdkArchive <path>'
        Show-Usage
        exit 1
    }

    if (-not $env:VCPKG_ROOT) {
        Write-LogError 'vcpkg package manager not found'
        Write-LogError 'Please set the VCPKG_ROOT environment variable'
        Write-LogError 'See: https://vcpkg.io/en/getting-started.html'
        exit 1
    }
    Write-LogInfo "VCPKG_ROOT: $env:VCPKG_ROOT"

    if (-not (Test-Command 'cmake')) {
        Write-LogError 'cmake command not found'
        Write-LogError 'Please install CMake: https://cmake.org/download/'
        exit 1
    }
    Write-LogInfo "Found cmake: $((& cmake --version | Select-Object -First 1))"

    if ($Jobs -eq 'auto') {
        $Script:JobCount = [System.Environment]::ProcessorCount
    } elseif ($Jobs -match '^[1-9][0-9]*$') {
        $Script:JobCount = [int]$Jobs
    } else {
        Write-LogError "Invalid -Jobs value '$Jobs'. Must be 'auto' or a positive integer."
        Show-Usage
        exit 1
    }
    Write-LogInfo "Parallel jobs: $($Script:JobCount)"

    if ($WithSanitizer) {
        if ($Toolchain -eq 'LLVM') {
            Write-LogError 'Sanitizer builds are not supported with the LLVM toolchain.'
            Write-LogError 'The only Windows sanitizer preset (windows-glfw-vs2022-address-sanitizer) uses MSVC,'
            Write-LogError 'and clang-cl AddressSanitizer is incompatible with the Debug CRT (see build_windows_sdk.ps1).'
            Write-LogError 'Use -Toolchain MSVC for sanitizer builds.'
            exit 1
        }
        Write-LogInfo "Sanitizer enabled: $WithSanitizer"
        Write-LogInfo 'Sanitizer builds are always RelWithDebInfo (-BuildType is ignored).'
    }

    if ($Toolchain -eq 'LLVM') {
        Write-LogInfo 'Toolchain: LLVM (clang-cl + lld-link)'
        Test-LlvmPrerequisites
    } elseif ($Generator -eq 'ninja' -and -not $WithSanitizer) {
        Test-NinjaPrerequisites
    } else {
        Test-VisualStudioPrerequisites
    }

    Write-LogSuccess 'Prerequisites check passed'

    Expand-SdkArchive

    Build-AllExamples
}

try {
    Invoke-Main
} catch {
    $invocationInfo = $_.InvocationInfo
    if ($invocationInfo -and $invocationInfo.ScriptLineNumber) {
        $failedLine = ''
        if ($invocationInfo.Line) {
            $failedLine = $invocationInfo.Line.Trim()
        }
        Write-LogError "Command failed at line $($invocationInfo.ScriptLineNumber): $failedLine"
    }
    Write-LogError $_.Exception.Message
    $Script:ExitCode = 1
} finally {
    Set-PSDebug -Trace 0

    Invoke-DistClean

    if ($Script:ShowExitMessage) {
        Write-Host ''
        Write-LogInfo 'Bye-Bye'
    }
}

exit $Script:ExitCode
