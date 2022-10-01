# Tracker for FPV

# How to setup build and debug tools youtube video from EmbeddedGeek

STM32 toolchain for Windows - Part 1 (CubeMX, GCC, Make and OpenOCD)
https://www.youtube.com/watch?v=PxQw5_7yI8Q

Visual Studio Code for STM32 development and debugging - Part 2
https://www.youtube.com/watch?v=xaC5oWwzOt0&t=701s

# downloads

## GNU Arm Embedded Toolchain

https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
gcc-arm-none-eabi-9-2020-q2-update-win32.zip

## Make for Windows

Binaries
http://gnuwin32.sourceforge.net/downlinks/make-bin-zip.php

Dependencies
http://gnuwin32.sourceforge.net/downlinks/make-dep-zip.php

## OpenOCD

https://gnutoolchains.com/arm-eabi/openocd/
https://sysprogs.com/getfile/1180/openocd-20200729.7z

# set env

ARMGCC_DIR: c:\Tools\gcc-arm-none-eabi-10.3-2021.10\bin\

# Compile and run from [VS Code] terminal

use Cortex-Debug 1.4.4 to avoid error "GDB major version should be >= 9, yours is 8"

```
make flash
```

# SIM800L initial configuration - Set Command Echo Mode OFF - run once for new SIM800L

ATE0
AT&W

# test SIM800L with FTDI

set FTDI to 3.3V

## wiring

```
FTDI SIM800L

GNG - GND
TXD - RXD
RXD - TXD

Connect SIM800L to Power supply 4.1V
```

use PuTTY

Connection type: Serial
Serial line: see in device manager
Speed: 9600
Terminal -> Line discipline options

- Local echo: Force on
- Local line editing: Force on

AT
AT+CSTT="TM"
AT+CIICR
AT+CIFSR
AT+CIPSTART="TCP","mail-verif.com",20300

AT+CIPSEND=5
HELLO

AT+CIPCLOSE
AT+CIPSHUT

// check status
AT+CIPSTATUS

// check voltage
AT+CBC

# SIM800L - commands reference

// Set TA Response Format - result codes
ATV0
