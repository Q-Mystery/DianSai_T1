param(
    [string]$SdkRoot = $env:COM_TI_MSPM0_SDK_INSTALL_DIR,
    [string]$CompilerRoot = $env:TI_ARM_CLANG_ROOT,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "Debug"
$ProjectName = "eight_ir_oled_new"

function Resolve-Tool {
    param(
        [string]$Name,
        [string]$Root,
        [string]$Relative
    )

    if ($Root) {
        $candidate = Join-Path $Root $Relative
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }

    return $null
}

function Find-First {
    param([string[]]$Candidates)

    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path $candidate)) {
            return (Resolve-Path $candidate).Path
        }
    }
    return $null
}

if ($Clean) {
    if (Test-Path $BuildDir) {
        Remove-Item -LiteralPath $BuildDir -Recurse -Force
    }
    Write-Host "Cleaned $BuildDir"
    exit 0
}

if (-not $SdkRoot) {
    $SdkRoot = Find-First @(
        "D:\ti\mspm0_sdk_2_11_00_07",
        "D:\ti\mspm0_sdk_2_02_00_05",
        "C:\ti\mspm0_sdk_2_02_00_05",
        "C:\ti\mspm0_sdk_2_05_00_05",
        "C:\ti\mspm0_sdk_2_04_00_06",
        "C:\ti\mspm0_sdk_2_03_00_07"
    )
}

if (-not $SdkRoot -or -not (Test-Path (Join-Path $SdkRoot "source\ti\devices\msp\msp.h"))) {
    throw "MSPM0 SDK not found. Set COM_TI_MSPM0_SDK_INSTALL_DIR or pass -SdkRoot C:\ti\mspm0_sdk_2_02_00_05."
}

$compilerExe = Resolve-Tool "tiarmclang.exe" $CompilerRoot "bin\tiarmclang.exe"
if (-not $compilerExe) {
    $compilerExe = Get-ChildItem -Path @(
        "D:\ti\ccstheia*\ccs\tools\compiler\ti-cgt-armllvm*\bin\tiarmclang.exe",
        "D:\ti\ccstheia*\tools\compiler\ti-cgt-armllvm*\bin\tiarmclang.exe",
        "D:\ti\ccs*\ccs\tools\compiler\ti-cgt-armllvm*\bin\tiarmclang.exe",
        "D:\ti\ccs*\tools\compiler\ti-cgt-armllvm*\bin\tiarmclang.exe",
        "C:\ti\ccs*\ccs\tools\compiler\ti-cgt-armllvm*\bin\tiarmclang.exe",
        "C:\ti\ccs*\tools\compiler\ti-cgt-armllvm*\bin\tiarmclang.exe",
        "C:\ti\ccstheia*\ccs\tools\compiler\ti-cgt-armllvm*\bin\tiarmclang.exe",
        "C:\ti\ccstheia*\tools\compiler\ti-cgt-armllvm*\bin\tiarmclang.exe"
    ) -ErrorAction SilentlyContinue |
        Select-Object -First 1 -ExpandProperty FullName
}
if (-not $compilerExe) {
    throw "tiarmclang.exe not found. Open this folder in CCS Theia or set TI_ARM_CLANG_ROOT to the TI Arm Clang toolchain root."
}

$compilerBin = Split-Path -Parent $compilerExe
$objcopyExe = Join-Path $compilerBin "tiarmobjcopy.exe"
$sizeExe = Join-Path $compilerBin "tiarmsize.exe"

$startup = Find-First @(
    (Join-Path $SdkRoot "source\ti\devices\msp\m0p\startup_system_files\ticlang\startup_mspm0g350x_ticlang.c"),
    (Join-Path $SdkRoot "source\ti\devices\msp\m0p\startup_system_files\ticlang\startup_mspm0g350x.c")
)
if (-not $startup) {
    throw "MSPM0G350x ticlang startup file not found under $SdkRoot\source\ti\devices\msp\m0p\startup_system_files\ticlang."
}

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

$sources = @(
    "main.c",
    "ti_msp_dl_config.c",
    "BSP\delay.c",
    "BSP\usart0.c",
    "BSP\bsp_beep_led.c",
    "BSP\bsp_timer.c",
    "BSP\bsp_ir_eight.c",
    "BSP\Motor\bsp_encoder.c",
    "BSP\Motor\bsp_motor.c",
    "BSP\OLED\oled.c",
    "APP\app_bcd_display.c",
    "APP\app_motor.c",
    "APP\bsp_PID_motor.c",
    "APP\app_irtracking_eight.c",
    "APP\app_imu.c",
    "APP\app_status_display.c",
    "APP\app_ultrasonic.c",
    "APP\app_voice.c"
)

$includeDirs = @(
    $ProjectRoot,
    (Join-Path $ProjectRoot "BSP"),
    (Join-Path $ProjectRoot "BSP\Motor"),
    (Join-Path $ProjectRoot "BSP\OLED"),
    (Join-Path $ProjectRoot "APP"),
    (Join-Path $SdkRoot "source"),
    (Join-Path $SdkRoot "source\third_party\CMSIS\Core\Include"),
    (Join-Path $SdkRoot "source\ti\driverlib\m0p\sysctl")
)

$commonArgs = @(
    "--target=arm-ti-none-eabi",
    "-mcpu=cortex-m0plus",
    "-mthumb",
    "-mlittle-endian",
    "-mfloat-abi=soft",
    "-Oz",
    "-g",
    "-std=c99",
    "-ffunction-sections",
    "-fdata-sections",
    "-fshort-enums",
    "-funsigned-char",
    "-D__MSPM0G3507__"
)

$includeArgs = @()
foreach ($dir in $includeDirs) {
    $includeArgs += "-I$dir"
}

$objects = New-Object System.Collections.Generic.List[string]
$allSources = @($startup) + ($sources | ForEach-Object { Join-Path $ProjectRoot $_ })

foreach ($src in $allSources) {
    if (-not (Test-Path $src)) {
        throw "Source file not found: $src"
    }

    $relative = if ($src.StartsWith($ProjectRoot)) {
        $src.Substring($ProjectRoot.Length).TrimStart("\", "/")
    } else {
        Split-Path -Leaf $src
    }

    $obj = Join-Path $BuildDir ($relative -replace "[:\\/]", "_")
    $obj = [System.IO.Path]::ChangeExtension($obj, ".o")
    $dep = [System.IO.Path]::ChangeExtension($obj, ".d")
    $objects.Add($obj)

    & $compilerExe @commonArgs @includeArgs -MMD -MP -MF $dep -c $src -o $obj
    if ($LASTEXITCODE -ne 0) {
        throw "Compile failed: $src"
    }
}

$rsp = Join-Path $BuildDir "objects.rsp"
Set-Content -Path $rsp -Value ($objects | ForEach-Object { '"' + ($_.Replace("\", "/")) + '"' }) -Encoding ASCII

$outFile = Join-Path $BuildDir "$ProjectName.out"
$mapFile = Join-Path $BuildDir "$ProjectName.map"
$cmdFile = Join-Path $ProjectRoot "device_linker.cmd"
$driverLib = Join-Path $SdkRoot "source\ti\driverlib\lib\ticlang\m0p\mspm0g1x0x_g3x0x\driverlib.a"

if (-not (Test-Path $driverLib)) {
    throw "DriverLib archive not found: $driverLib"
}

& $compilerExe @commonArgs "@$rsp" $cmdFile $driverLib `
    "-Wl,-m$mapFile" `
    "-Wl,--rom_model" `
    "-o" $outFile
if ($LASTEXITCODE -ne 0) {
    throw "Link failed"
}

if (Test-Path $objcopyExe) {
    & $objcopyExe -O ihex $outFile (Join-Path $BuildDir "$ProjectName.hex")
    if ($LASTEXITCODE -ne 0) {
        throw "Hex conversion failed"
    }
}

if (Test-Path $sizeExe) {
    & $sizeExe $outFile
    if ($LASTEXITCODE -ne 0) {
        throw "Size report failed"
    }
}

Write-Host "Built $outFile"
