# How to Use an ILI9341 TFT LCD with Zephyr on Raspberry Pi Pico 2

## Introduction

This document describes how to use an SPI-connected TFT LCD with Zephyr on a Raspberry Pi Pico 2.

The goal is to enable the Display Subsystem, LVGL, and a Touch Screen.

## Development Environment

- macOS Tahoe 26.5.1
- Zephyr 4.4.0
- Raspberry Pi Pico2
- 2.8-inch TFT LCD Module, ILI9341 Controller, SPI Interface, 240×320

## Connecting the TFT LCD

The TFT LCD is connected to **SPI0** on the Raspberry Pi Pico 2. Several additional signals must also be connected to GPIO pins.

The wiring is as follows. Since the SPI0 signals can be assigned to multiple GPIO pins, be careful to use the correct pins. The TFT LCD signal names correspond to the labels printed on the PCB.

| Pico 2          | TFT LCD    |
| :-------------- | :--------- |
| SPI0_CSn (GP17) | CS         |
| GP22            | RESET      |
| GP21            | DC         |
| SPI0_TX (GP19)  | SDI (MOSI) |
| SPI0_SCK (GP18) | SCK        |
| GP20            | LED        |
| SPI0_RX (GP16)  | SDO (MISO) |
| SPI0_SCK (GP18) | T_CLK      |
| GP27            | T_CS       |
| SPI0_TX (GP19)  | T_DIN      |
| SPI0_RX (GP16)  | T_DO       |
| GP26            | T_IRQ      |

Both the TFT LCD and the touch screen use SPI communication. In this configuration, the three SPI signals (SPI0_SCK, SPI0_TX, and SPI0_RX) are shared and connected to SPI0.

The TFT LCD's VCC and GND pins are connected to the breakout board. An AC adapter is connected to the breakout board, which is configured to provide 3.3 V.

Since the TFT LCD consumes a reasonable amount of power, an external power source is used via the breakout board for safety. I have not tested powering the TFT LCD directly from the Pico 2's 3.3 V output.

As shown in the image, the TFT LCD is mounted with the connector positioned at the top of the display.

<img title="" src="images/PICO2-ILI9341-SPI.jpg">

# Preparing the Zephyr Build Environment

Follow the steps in the official Getting Started Guide to set up the build environment.

This document assumes that Zephyr has been installed under:

```text
$HOME/zephyrproject/
```

# Obtaining the Source Code

Clone the source code repository from GitHub. In this example, it is cloned under `$HOME`.

```bash
cd $HOME
git clone https://github.com/hkato-sssl/rpi-pico2-zephyr-ili9341-spi.git
```

The following three files are used throughout this document:

* rpi_pico2_rp2350a_m33.conf
* rpi_pico2_rp2350a_m33.overlay
* rpi_pico2_rp2350a_m33/init_ili9341_backlight.c

## rpi_pico2_rp2350a_m33.conf

This file contains additional Kconfig settings for the Raspberry Pi Pico 2.

The added configuration enables:

* USB UART support
* SPI and GPIO support
* XPT2046 touch screen support
* Byte swapping for 16-bit LVGL pixel data

## rpi_pico2_rp2350a_m33.overlay

This file defines Raspberry Pi Pico 2 specific Devicetree settings, including:

* USB UART configuration
* TFT LCD configuration
* Touch screen configuration

The communication frequency with the ILI9341 is set to 40 MHz. In my environment, operation remained stable up to 70 MHz. The ability to run at such a high frequency may be partly due to the external power supply.

The ILI9341 Devicetree supports display rotation and other options. In this article, the display orientation is configured to match the touch screen orientation. Since the XPT2046 appears to provide fewer orientation settings, the display configuration was adjusted instead.

## init_ili9341_backlight.c

This file implements TFT LCD backlight initialization.

Currently, backlight control is not integrated into the Zephyr display subsystem, so it must be implemented as board-specific code. In this example, a GPIO pin is used to drive the TFT LCD LED signal high.

Functions registered with `SYS_INIT()` are executed before `main()`. By using `SYS_INIT()`, board-specific initialization can be added without modifying the sample application's source code.

# Building and Testing

The following sections use Zephyr sample applications to verify TFT LCD operation.

First, initialize the Zephyr build environment:

```bash
cd $HOME
source zephyrproject/.venv/bin/activate
source zephyrproject/zephyr/zephyr-env.sh
```

## Display Subsystem

This section verifies the operation of the Display Subsystem.

Build the sample as follows:

```bash
cd $HOME/zephyrproject/zephyr/samples/drivers/display
mkdir -p boards && cp -r $HOME/rpi-pico2-zephyr-ili9341-spi/rpi_pico2_rp2350a_m33* boards
echo "target_sources(app PRIVATE boards/rpi_pico2_rp2350a_m33/init_ili9341_backlight.c)" >> CMakeLists.txt
west build -p -b rpi_pico2/rp2350a/m33
```

This copies the Raspberry Pi Pico 2 specific `.conf`, `.overlay`, and source files into the `boards` directory and updates `CMakeLists.txt`.

The sample itself works once the correct configuration and Devicetree are provided. However, `CMakeLists.txt` must be modified to include the backlight initialization source file.

After the build completes, hold down the **BOOTSEL** button while powering on the Pico 2, then flash the image:

```bash
west flash -r uf2
```

If the display appears as shown in the official documentation, the setup is working correctly. 

## Touch Screen

Next, verify touch screen functionality using the `draw_touch_events` sample.

Build the sample:

```bash
cd $HOME/zephyrproject/zephyr/samples/subsys/input/draw_touch_events
mkdir -p boards && cp -r $HOME/rpi-pico2-zephyr-ili9341-spi/rpi_pico2_rp2350a_m33* boards
echo "target_sources(app PRIVATE boards/rpi_pico2_rp2350a_m33/init_ili9341_backlight.c)" >> CMakeLists.txt
west build -p -b rpi_pico2/rp2350a/m33
```

After building, hold down **BOOTSEL** while powering on the Pico 2 and flash the image:

```bash
west flash -r uf2
```

This sample displays a "+" symbol at the touched position.

Depending on your hardware, the displayed position may be offset from the actual touch point. In such cases, touch screen calibration is required.

## LVGL

Finally, run an LVGL sample application.

Build the demo:

```bash
cd $HOME/zephyrproject/zephyr/samples/modules/lvgl/demos
mkdir -p boards && cp -r $HOME/rpi-pico2-zephyr-ili9341-spi/rpi_pico2_rp2350a_m33* boards
echo "target_sources(app PRIVATE boards/rpi_pico2_rp2350a_m33/init_ili9341_backlight.c)" >> CMakeLists.txt
west build -p -b rpi_pico2/rp2350a/m33 -- -DCONFIG_LV_Z_DEMO_MUSIC=y
```

After building, hold down **BOOTSEL** while powering on the Pico 2 and flash the image:

```bash
west flash -r uf2
```

The demo can be changed by replacing the final `-Dxxxx` option in the `west build` command with one of the following:

```text
-DCONFIG_LV_Z_DEMO_MUSIC=y
-DCONFIG_LV_Z_DEMO_BENCHMARK=y
-DCONFIG_LV_Z_DEMO_STRESS=y
-DCONFIG_LV_Z_DEMO_WIDGETS=y
-DCONFIG_LV_Z_DEMO_KEYPAD_AND_ENCODER=y
-DCONFIG_LV_Z_DEMO_RENDER=y
```

## Related Article

https://qiita.com/hkato-sssl/items/e95881f889e9d5217201
