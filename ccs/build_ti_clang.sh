#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/Debug"
PROJECT_NAME="eight_ir_oled_new"

find_first_dir() {
    for candidate in "$@"; do
        if [ -n "$candidate" ] && [ -d "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    return 1
}

find_first_file() {
    for candidate in "$@"; do
        if [ -n "$candidate" ] && [ -f "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    return 1
}

if [ "${1:-}" = "clean" ]; then
    rm -rf "$BUILD_DIR"
    echo "Cleaned $BUILD_DIR"
    exit 0
fi

SDK_ROOT="${COM_TI_MSPM0_SDK_INSTALL_DIR:-${MSPM0_SDK_ROOT:-}}"
if [ -z "$SDK_ROOT" ]; then
    SDK_ROOT="$(find_first_dir \
        "$PROJECT_ROOT/MSPM0G3507_H_Car_Keil111/MSPM0G3507_H_Car_Keil/SDK" \
        /Applications/ti/mspm0_sdk_* \
        /opt/ti/mspm0_sdk_* \
        "$HOME/ti/mspm0_sdk_*" \
        2>/dev/null || true)"
fi

if [ -z "$SDK_ROOT" ] || [ ! -f "$SDK_ROOT/source/ti/devices/msp/msp.h" ]; then
    echo "MSPM0 SDK not found. Set COM_TI_MSPM0_SDK_INSTALL_DIR." >&2
    exit 1
fi

COMPILER_ROOT="${TI_ARM_CLANG_ROOT:-${TI_CGT_ARMLLVM_ROOT:-}}"
TIARMCLANG=""
if [ -n "$COMPILER_ROOT" ]; then
    TIARMCLANG="$(find_first_file "$COMPILER_ROOT/bin/tiarmclang" \
        "$COMPILER_ROOT/bin/tiarmclang.exe" 2>/dev/null || true)"
fi
if [ -z "$TIARMCLANG" ]; then
    TIARMCLANG="$(find_first_file \
        /Applications/ti/ccs*/ccs/tools/compiler/ti-cgt-armllvm*/bin/tiarmclang \
        /opt/ti/ccs*/ccs/tools/compiler/ti-cgt-armllvm*/bin/tiarmclang \
        "$HOME/ti/ccs*/ccs/tools/compiler/ti-cgt-armllvm*/bin/tiarmclang" \
        2>/dev/null || true)"
fi
if [ -z "$TIARMCLANG" ] && command -v tiarmclang >/dev/null 2>&1; then
    TIARMCLANG="$(command -v tiarmclang)"
fi
if [ -z "$TIARMCLANG" ]; then
    echo "tiarmclang not found. Set TI_ARM_CLANG_ROOT." >&2
    exit 1
fi

COMPILER_BIN="$(cd "$(dirname "$TIARMCLANG")" && pwd)"
TIARMOBJCOPY="$COMPILER_BIN/tiarmobjcopy"
TIARMSIZE="$COMPILER_BIN/tiarmsize"

STARTUP="$(find_first_file \
    "$SDK_ROOT/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c" \
    "$SDK_ROOT/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x.c")"
DRIVERLIB="$SDK_ROOT/source/ti/driverlib/lib/ticlang/m0p/mspm0g1x0x_g3x0x/driverlib.a"
if [ ! -f "$DRIVERLIB" ]; then
    echo "DriverLib archive not found: $DRIVERLIB" >&2
    exit 1
fi

mkdir -p "$BUILD_DIR"

COMMON_ARGS=(
    --target=arm-ti-none-eabi
    -mcpu=cortex-m0plus
    -mthumb
    -mlittle-endian
    -mfloat-abi=soft
    -Oz
    -g
    -std=c99
    -ffunction-sections
    -fdata-sections
    -fshort-enums
    -funsigned-char
    -D__MSPM0G3507__
)

INCLUDE_ARGS=(
    "-I$PROJECT_ROOT"
    "-I$PROJECT_ROOT/BSP"
    "-I$PROJECT_ROOT/BSP/Motor"
    "-I$PROJECT_ROOT/BSP/OLED"
    "-I$PROJECT_ROOT/APP"
    "-I$SDK_ROOT/source"
    "-I$SDK_ROOT/source/third_party/CMSIS/Core/Include"
    "-I$SDK_ROOT/source/ti/driverlib/m0p/sysctl"
)

SOURCES=(
    "$STARTUP"
    "$PROJECT_ROOT/main.c"
    "$PROJECT_ROOT/ti_msp_dl_config.c"
    "$PROJECT_ROOT/BSP/delay.c"
    "$PROJECT_ROOT/BSP/usart0.c"
    "$PROJECT_ROOT/BSP/bsp_beep_led.c"
    "$PROJECT_ROOT/BSP/bsp_timer.c"
    "$PROJECT_ROOT/BSP/bsp_ir_eight.c"
    "$PROJECT_ROOT/BSP/Motor/bsp_encoder.c"
    "$PROJECT_ROOT/BSP/Motor/bsp_motor.c"
    "$PROJECT_ROOT/BSP/OLED/oled.c"
    "$PROJECT_ROOT/APP/app_bcd_display.c"
    "$PROJECT_ROOT/APP/app_motor.c"
    "$PROJECT_ROOT/APP/bsp_PID_motor.c"
    "$PROJECT_ROOT/APP/app_irtracking_eight.c"
    "$PROJECT_ROOT/APP/app_imu.c"
    "$PROJECT_ROOT/APP/app_status_display.c"
    "$PROJECT_ROOT/APP/app_ultrasonic.c"
    "$PROJECT_ROOT/APP/app_voice.c"
)

OBJECTS=()
index=0
for source in "${SOURCES[@]}"; do
    object="$BUILD_DIR/source_${index}.o"
    dep="$BUILD_DIR/source_${index}.d"
    "$TIARMCLANG" "${COMMON_ARGS[@]}" "${INCLUDE_ARGS[@]}" \
        -MMD -MP -MF "$dep" -c "$source" -o "$object"
    OBJECTS+=("$object")
    index=$((index + 1))
done

OUT_FILE="$BUILD_DIR/$PROJECT_NAME.out"
HEX_FILE="$BUILD_DIR/$PROJECT_NAME.hex"
MAP_FILE="$BUILD_DIR/$PROJECT_NAME.map"

"$TIARMCLANG" "${COMMON_ARGS[@]}" "${OBJECTS[@]}" \
    "$PROJECT_ROOT/device_linker.cmd" "$DRIVERLIB" \
    "-Wl,-m$MAP_FILE" "-Wl,--rom_model" -o "$OUT_FILE"

if [ -x "$TIARMOBJCOPY" ]; then
    "$TIARMOBJCOPY" -O ihex "$OUT_FILE" "$HEX_FILE"
else
    echo "tiarmobjcopy not found at $TIARMOBJCOPY" >&2
    exit 1
fi

if [ -x "$TIARMSIZE" ]; then
    "$TIARMSIZE" "$OUT_FILE"
fi

echo "Built $OUT_FILE"
echo "Built $HEX_FILE"
