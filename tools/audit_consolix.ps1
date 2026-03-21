param(
    [string]$Compiler = "g++",
    [switch]$KeepTemp
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $false

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$TempRoot = Join-Path $env:TEMP ("consolix-audit-" + [guid]::NewGuid().ToString())
New-Item -ItemType Directory -Path $TempRoot | Out-Null

$IncludeArgs = @(
    "-I$RepoRoot/include",
    "-I$RepoRoot/libs/cxxopts/include",
    "-I$RepoRoot/libs/json/include",
    "-I$RepoRoot/libs/log-it-cpp/include/logit_cpp",
    "-I$RepoRoot/libs/time-shield-cpp/include/time_shield_cpp"
)

$Results = [System.Collections.Generic.List[object]]::new()

function Get-Headline {
    param([string]$Text)

    if ([string]::IsNullOrWhiteSpace($Text)) {
        return ""
    }

    foreach ($line in ($Text -split "(`r`n|`n|`r)")) {
        $trimmed = $line.Trim()
        if ($trimmed.Length -gt 0) {
            return $trimmed
        }
    }

    return ""
}

function Invoke-Native {
    param(
        [string]$FilePath,
        [string[]]$Arguments
    )

    $previousPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        $output = & $FilePath @Arguments 2>&1
        $exitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousPreference
    }

    return [pscustomobject]@{
        ExitCode = $exitCode
        Output   = (($output | ForEach-Object { "$_" }) -join "`n")
    }
}

function Add-Result {
    param(
        [string]$Category,
        [string]$Name,
        [string]$Status,
        [string]$Detail,
        [string]$Output
    )

    $Results.Add([pscustomobject]@{
        Category = $Category
        Name     = $Name
        Status   = $Status
        Detail   = $Detail
        Summary  = (Get-Headline $Output)
        Output   = $Output
    })
}

function Write-SourceFile {
    param(
        [string]$Path,
        [string]$Content
    )

    Set-Content -Path $Path -Value $Content -Encoding utf8
}

function Invoke-CompileCheck {
    param(
        [string]$Category,
        [string]$Name,
        [string]$Standard,
        [string[]]$Defines,
        [string]$Source
    )

    $dir = Join-Path $TempRoot ([guid]::NewGuid().ToString())
    New-Item -ItemType Directory -Path $dir | Out-Null
    $src = Join-Path $dir "main.cpp"
    Write-SourceFile -Path $src -Content $Source

    $defineArgs = @()
    foreach ($define in $Defines) {
        $defineArgs += "-D$define"
    }

    $result = Invoke-Native -FilePath $Compiler -Arguments (@("-std=$Standard", "-fsyntax-only") + $IncludeArgs + $defineArgs + @($src))
    $status = if ($result.ExitCode -eq 0) { "PASS" } else { "FAIL" }
    $detail = if ($Defines.Count -gt 0) {
        "$Standard | $($Defines -join ', ')"
    } else {
        $Standard
    }

    Add-Result -Category $Category -Name $Name -Status $status -Detail $detail -Output $result.Output
}

function Invoke-OdrCheck {
    param(
        [string]$Name,
        [string[]]$Defines,
        [string]$TranslationUnitBody
    )

    $dir = Join-Path $TempRoot ([guid]::NewGuid().ToString())
    New-Item -ItemType Directory -Path $dir | Out-Null

    $aPath = Join-Path $dir "a.cpp"
    $bPath = Join-Path $dir "b.cpp"
    $mainPath = Join-Path $dir "main.cpp"
    $aObj = Join-Path $dir "a.o"
    $bObj = Join-Path $dir "b.o"
    $mainObj = Join-Path $dir "main.o"
    $exePath = Join-Path $dir "audit.exe"

    $bodyA = @"
$TranslationUnitBody
int fa() { return 1; }
"@
    $bodyB = @"
$TranslationUnitBody
int fb() { return 2; }
"@
    $bodyMain = @"
int fa();
int fb();
int main() { return fa() + fb(); }
"@

    Write-SourceFile -Path $aPath -Content $bodyA
    Write-SourceFile -Path $bPath -Content $bodyB
    Write-SourceFile -Path $mainPath -Content $bodyMain

    $defineArgs = @()
    foreach ($define in $Defines) {
        $defineArgs += "-D$define"
    }

    $compileA = Invoke-Native -FilePath $Compiler -Arguments (@("-std=c++17", "-c") + $IncludeArgs + $defineArgs + @($aPath, "-o", $aObj))
    $compileB = Invoke-Native -FilePath $Compiler -Arguments (@("-std=c++17", "-c") + $IncludeArgs + $defineArgs + @($bPath, "-o", $bObj))
    $compileMain = Invoke-Native -FilePath $Compiler -Arguments (@("-std=c++17", "-c") + $IncludeArgs + @($mainPath, "-o", $mainObj))

    $combinedOutput = @(
        "compile a.cpp"
        $compileA.Output
        ""
        "compile b.cpp"
        $compileB.Output
        ""
        "compile main.cpp"
        $compileMain.Output
    ) -join "`n"

    if ($compileA.ExitCode -ne 0 -or $compileB.ExitCode -ne 0 -or $compileMain.ExitCode -ne 0) {
        Add-Result -Category "ODR" -Name $Name -Status "FAIL" -Detail "c++17" -Output $combinedOutput
        return
    }

    $link = Invoke-Native -FilePath $Compiler -Arguments @($aObj, $bObj, $mainObj, "-o", $exePath)
    $combinedOutput = $combinedOutput + "`n`nlink`n" + $link.Output
    $status = if ($link.ExitCode -eq 0) { "PASS" } else { "FAIL" }
    Add-Result -Category "ODR" -Name $Name -Status $status -Detail "c++17" -Output $combinedOutput
}

function Invoke-CMakeBuildCheck {
    param(
        [string]$Standard
    )

    $buildDir = Join-Path $TempRoot "cmake-build-$Standard"
    $configure = Invoke-Native -FilePath "cmake" -Arguments @(
        "-S", $RepoRoot.Path,
        "-B", $buildDir,
        "-G", "MinGW Makefiles",
        "-DCONSOLIX_CXX_STANDARD=$Standard"
    )
    $output = "configure`n$($configure.Output)"
    if ($configure.ExitCode -eq 0) {
        $build = Invoke-Native -FilePath "cmake" -Arguments @("--build", $buildDir)
        $output = $output + "`n`nbuild`n" + $build.Output
        $status = if ($build.ExitCode -eq 0) { "PASS" } else { "FAIL" }
        Add-Result -Category "Build" -Name "Root CMake examples" -Status $status -Detail "MinGW Makefiles | C++$Standard" -Output $output
    } else {
        Add-Result -Category "Build" -Name "Root CMake examples" -Status "FAIL" -Detail "MinGW Makefiles | C++$Standard" -Output $output
    }
}

$AllOff = @(
    "CONSOLIX_USE_LOGIT=0",
    "CONSOLIX_USE_CXXOPTS=0",
    "CONSOLIX_USE_JSON=0"
)
$JsonOnly = @(
    "CONSOLIX_USE_LOGIT=0",
    "CONSOLIX_USE_CXXOPTS=0",
    "CONSOLIX_USE_JSON=1"
)
$CxxoptsOnly = @(
    "CONSOLIX_USE_LOGIT=0",
    "CONSOLIX_USE_CXXOPTS=1",
    "CONSOLIX_USE_JSON=0"
)
$LogitOnly = @(
    "CONSOLIX_USE_LOGIT=1",
    "CONSOLIX_USE_CXXOPTS=0",
    "CONSOLIX_USE_JSON=0"
)
$AllOn = @(
    "CONSOLIX_USE_LOGIT=1",
    "CONSOLIX_USE_CXXOPTS=1",
    "CONSOLIX_USE_JSON=1"
)

$EntryHeaders = @(
    "consolix/consolix.hpp",
    "consolix/core.hpp",
    "consolix/components.hpp",
    "consolix/utils.hpp"
)

$StandaloneChecks = @(
    [pscustomobject]@{
        Name     = "consolix/utils/json_utils.hpp"
        Defines  = $AllOff
        Source   = @"
#include <string>
#include <consolix/utils/json_utils.hpp>
int main() {
    auto s = consolix::strip_json_comments("{\"a\":1}");
    return static_cast<int>(s.size());
}
"@
    },
    [pscustomobject]@{
        Name     = "consolix/utils/path_utils.hpp"
        Defines  = $AllOff
        Source   = @"
#include <consolix/utils/path_utils.hpp>
int main() {
    auto s = consolix::get_exec_dir();
    return static_cast<int>(s.size());
}
"@
    },
    [pscustomobject]@{
        Name     = "consolix/utils/enums.hpp"
        Defines  = $AllOff
        Source   = @"
#include <consolix/utils/enums.hpp>
int main() {
    auto color = consolix::TextColor::Blue;
    return static_cast<int>(color);
}
"@
    },
    [pscustomobject]@{
        Name     = "consolix/utils/types.hpp"
        Defines  = $CxxoptsOnly
        Source   = @"
#include <consolix/utils/types.hpp>
int main() {
    consolix::CliOptions options("audit", "types");
    return 0;
}
"@
    }
)

foreach ($standard in @("c++11", "c++14", "c++17")) {
    foreach ($header in $EntryHeaders) {
        Invoke-CompileCheck -Category "EntryPoints" -Name $header -Standard $standard -Defines $AllOff -Source @"
#include <$header>
int main() { return 0; }
"@
    }
}

foreach ($scenario in @(
    [pscustomobject]@{ Name = "all off"; Defines = $AllOff },
    [pscustomobject]@{ Name = "json only"; Defines = $JsonOnly },
    [pscustomobject]@{ Name = "cxxopts only"; Defines = $CxxoptsOnly },
    [pscustomobject]@{ Name = "logit only"; Defines = $LogitOnly },
    [pscustomobject]@{ Name = "all on"; Defines = $AllOn }
)) {
    foreach ($header in $EntryHeaders) {
        Invoke-CompileCheck -Category "MacroMatrix" -Name $header -Standard "c++17" -Defines $scenario.Defines -Source @"
#include <$header>
int main() { return 0; }
"@
    }
}

foreach ($check in $StandaloneChecks) {
    Invoke-CompileCheck -Category "Standalone" -Name $check.Name -Standard "c++17" -Defines $check.Defines -Source $check.Source
}

Invoke-OdrCheck -Name "consolix/utils/json_utils.hpp" -Defines $AllOff -TranslationUnitBody @"
#include <string>
#include <consolix/utils/json_utils.hpp>
"@
Invoke-OdrCheck -Name "consolix/utils.hpp" -Defines $AllOff -TranslationUnitBody @"
#include <consolix/utils.hpp>
"@
Invoke-OdrCheck -Name "consolix/core.hpp" -Defines $AllOff -TranslationUnitBody @"
#include <consolix/core.hpp>
"@

foreach ($cmakeStandard in @("11", "14", "17")) {
    Invoke-CMakeBuildCheck -Standard $cmakeStandard
}

$Results |
    Sort-Object Category, Name, Detail |
    Select-Object Category, Name, Detail, Status, Summary |
    Format-Table -AutoSize

$FailCount = @($Results | Where-Object { $_.Status -eq "FAIL" }).Count
$PassCount = @($Results | Where-Object { $_.Status -eq "PASS" }).Count

Write-Host ""
Write-Host "Pass: $PassCount"
Write-Host "Fail: $FailCount"
Write-Host "Temp: $TempRoot"

if (-not $KeepTemp) {
    Remove-Item -Recurse -Force $TempRoot
}

if ($FailCount -gt 0) {
    exit 1
}
