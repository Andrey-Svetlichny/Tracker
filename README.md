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

# Compile and run from [VS Code] terminal

```
make flash
```


# SIM800L initial configuration - set parameters and save - run once for new SIM800L
/*
 sim800("ATE0"); // Set Command Echo Mode OFF - don't use ?
 sim800("ATV0"); // Set TA Response Format - result codes
 sim800("AT&W"); // Save
 */