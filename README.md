## Standalone Task 1 Firmware

This CCS project is permanently bound to competition task 1. It starts task 1
automatically after power-up initialization; no start switch is required. The
Debug build produces `Competition_PJ_Task1.out` and `Competition_PJ_Task1.hex`.

## Example Summary

Empty project using DriverLib.
This example shows a basic empty project using DriverLib with just main file
and SysConfig initialization.

### Low-Power Recommendations
TI recommends to terminate unused pins by setting the corresponding functions to
GPIO and configure the pins to output low or input with internal
pullup/pulldown resistor.

SysConfig allows developers to easily configure unused pins by selecting **Board**→**Configure Unused Pins**.

## Example Usage

Compile, load and run the example.

## Competition board mapping

This project targets **MSPM0G3507, LQFP-64** and uses the following direct
motor/encoder/grayscale connections:

| Signal | GPIO | Function |
| --- | --- | --- |
| PWMA / PWMD | PB12 / PB4 | TIMA0.CCP1 / TIMA0.CCP2 |
| AIN1 / AIN2 | PB15 / PB17 | Left motor direction |
| DIN1 / DIN2 | PA12 / PA13 | Right motor direction |
| E1A / E1B | PB8 / PB7 | Left quadrature encoder |
| E4A / E4B | PB20 / PB13 | Right quadrature encoder |
| AD2 / AD1 / AD0 | PB0 / PB6 / PB16 | Grayscale channel address |
| OUT | PB1 | Grayscale digital input |

The generated flash images are `Debug/Competition_PJ.out` and
`Debug/Competition_PJ.hex`. Motor speed-loop and odometry calibration values
are in `BSP/Motor/bsp_motor.h` and `BSP/Motor/bsp_motor.c`.
