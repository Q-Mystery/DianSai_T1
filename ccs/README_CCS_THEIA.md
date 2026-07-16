# CCS Theia migration notes

This folder adds a CCS Theia entry point for the current `IR_OLED` firmware.
The build is intentionally explicit: only the files used by the current
eight-channel IR/OLED target are compiled. Legacy K210, RGB, speech and IR
remote sources remain in the repository, but are not part of this target.

## Build in CCS Theia

1. Install Code Composer Studio Theia, TI Arm Clang, and MSPM0 SDK. This
   workspace is configured to auto-detect the local install under
   `D:\ti\mspm0_sdk_2_11_00_07` and `D:\ti\ccstheia100`.
2. Set the MSPM0 SDK path if CCS has not set it already:

   ```powershell
   $env:COM_TI_MSPM0_SDK_INSTALL_DIR = "D:\ti\mspm0_sdk_2_11_00_07"
   ```

3. Optional: set the TI Arm Clang root if `tiarmclang.exe` is not on `PATH`:

   ```powershell
   $env:TI_ARM_CLANG_ROOT = "D:\ti\ccstheia100\ccs\tools\compiler\ti-cgt-armllvm_2.1.3.LTS"
   ```

4. Open this repository folder in CCS Theia with **File > Open Folder...**.
5. Run **Terminal > Run Build Task... > CCS Theia: build eight_ir_oled_new**.

The output is generated under `Debug/`:

- `Debug/eight_ir_oled_new.out`
- `Debug/eight_ir_oled_new.hex`
- `Debug/eight_ir_oled_new.map`

## Build from macOS/Linux shell

The repository also includes a shell wrapper around TI Arm Clang:

```bash
./ccs/build_ti_clang.sh
```

It auto-detects TI Arm Clang under `/Applications/ti/ccs*/ccs/tools/compiler`
and falls back to the SDK copy bundled with this project. Set
`TI_ARM_CLANG_ROOT` or `COM_TI_MSPM0_SDK_INSTALL_DIR` to override either path.

## Source list for this CCS target

- `main.c`
- `ti_msp_dl_config.c`
- `BSP/delay.c`
- `BSP/usart0.c`
- `BSP/bsp_beep_led.c`
- `BSP/bsp_timer.c`
- `BSP/bsp_ir_eight.c`
- `BSP/Motor/bsp_encoder.c`
- `BSP/Motor/bsp_motor.c`
- `BSP/OLED/oled.c`
- `APP/app_bcd_display.c`
- `APP/app_motor.c`
- `APP/bsp_PID_motor.c`
- `APP/app_irtracking_eight.c`
- `APP/app_track_mission.c`
- `APP/app_imu.c`
- `APP/app_status_display.c`
- `APP/app_ultrasonic.c`
- `APP/app_voice.c`
- MSPM0 SDK startup file:
  `source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c`

## SysConfig

`empty.syscfg` is kept in the project root. The repository currently builds
against the checked-in generated files `ti_msp_dl_config.c/h`, which avoids
duplicate generated sources when opening the folder directly in CCS Theia.

If you want CCS Theia to auto-generate SysConfig sources instead, copy the
generated files from `Debug/SysConfig` back into the project root or remove the
checked-in generated files from the build and place the generated include
directory before the project root in the include path.

## Debug and flashing

Use CCS Theia's target/debug flow to create a target configuration for
`MSPM0G3507` and your probe, then load `Debug/eight_ir_oled_new.out`.

The original Keil project used a CMSIS-DAP style debug setup. In CCS Theia,
select the probe that matches your hardware, commonly XDS110 on TI LaunchPads
or the probe supplied with your MSPM0 board.
