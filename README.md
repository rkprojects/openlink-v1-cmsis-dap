# OpenLink-V1 CMSIS-DAP Debug Unit

Project home page: [https://ravikiranb.com/projects/cmsis-dap-debug/](https://ravikiranb.com/projects/cmsis-dap-debug/)

Build your own simple low cost CMSIS-DAP Debug Unit with USB HID interface 
to debug any ARM Cortex based microcontroller.

## Get Hardware and Firmware Files

$ git clone https://github.com/rkprojects/openlink-v1-cmsis-dap.git


# Hardware

* 55x30 mm board size.
* Low cost, Low pin count MCU STM32F070F6P6.
* Micro USB type B connector.
* 10-pin cortex debug connector
* All GPIOs on Headers.

## Design Files and Gerber

Schematic and PCB Layout files are in folder *openlink-v1-cmsis-dap/hardware/*  
Gerber files are in folder *openlink-v1-cmsis-dap/hardware/gerber/*  
BOM file is in folder *openlink-v1-cmsis-dap/hardware/docs/*  

Schematic and PCB Layout designed with [DipTrace Version 3.3.1.3 (Freeware)](https://diptrace.com/)  

## Notes on DRC

Before sending gerber files to fabrication please cross check the design rules 
with your PCB manufacturer's specifications and rerun DRC.

Current DRC violations waved off:  

* J3.1 and J3.5 Drill to SMD pad clearance is set to 12 mil as per my PCB manufacturer's specifications however 
in manufacturer's review it was waved off.  
* Y1.1 and Y1.2: Footprint design of Y1 (crystal) is a hybrid one (through hole pad overlapping SMD pad) 
with possibility of mounting either SMD or through hole crystal. Preferred part is SMD, however 
if you wish to use through hole part then while soldering raise the height of crystal such that 
metal case does not touch SMD pads.

# Firmware

## Software Requirements

* GCC ARM Toolchain
* [Keil MDK for ARM](http://www.keil.com/products/arm/mdk.asp) if you wish to validate the debug unit. 
Lite edition is sufficient for this project. 
You could also use [MDK for STM32F0](https://www2.keil.com/stmicroelectronics-stm32/mdk)
* [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) to flash firmware in USB DFU mode.
* Tested on OS: Windows 10 Build 14393.

## Source Code

Firmware source codes are in folder *openlink-v1-cmsis-dap/firmware/*

Pre-built binaries are present in the *openlink-v1-cmsis-dap/firmware/build/* directory. 
If you wish to rebuild then you may have to modify the Makefile to correctly point 
to toolchain binaries and run make.


## Flash Firmware

* Close J5.1-2 pins with jumper cap to boot from System memory in USB DFU mode.
* Connect the debug unit with USB cable to PC host.
* Start STM32CubeProgrammer and select USB as bootloader interface option. 
	* Click **Refresh** icon - Device information fields will get updated.
	* Click **Connect**.
	* Click on **Open file** tab and select *openlink_v1_cmsis_dap.hex* file 
	in *openlink-v1-cmsis-dap/firmware/build/* directory.
	* Click **Download** to flash.
	* Post download click **Disconnect**.
* Open J5.1-2 pins and close J5.2-3 pins with jumper cap to boot from Flash memory.
* Reset the board with **Reset** switch. There is no need to power cycle the board.
* Board should now get detected as USB HID device, cross check this in Device manager.

## Validate Debug Unit

* CMSIS DAP Firmware Validation project is in 
folder *openlink-v1-cmsis-dap/firmware/Validation/MDK5*.
* Follow the instructions in the readme.txt file there.
* In the step where CMSIS-DAP debugger needs to be selected, select **OpenLink-V1 CMSIS-DAP**.
* For specific example with LPC4357 MCU based target 
board refer to [project home page](https://ravikiranb.com/projects/cmsis-dap-debug/).

# Pending Stuff

* JTAG interface of the debug unit is not yet tested as 
all the Cortex-M boards I have are designed with SWD debug port. 
However JTAG interface is completely implemented in firmware.
* SWO Trace capture is not yet implemented.
* Currently a test USB VID/PID: [1209/0001](http://pid.codes) is assigned which is meant
for only private testing. Unique USB VID/PID assignment is pending.
* Current 10-pin debug connector (J4) is of through hole type with 1.27mm pitch. Commonly
used connectors are of SMD type compatible with samtec FTSH-105- styles.


# License

Copyright 2019 Ravikiran Bukkasagara <contact@ravikiranb.com>

This project uses multiple licenses:

* ![Creative Commons License](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)  
**Schematic and PCB Layout** design files are licensed under a 
[Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/)
* **CMSIS DAP firmware code** is licensed under [Apache License, Version 2.0](www.apache.org/licenses/LICENSE-2.0)
* **STMicroelectronics supplied USB Middleware, HAL, LL drivers and startup source codes** are licensed 
under [Ultimate Liberty license SLA0044](www.st.com/SLA0044)
* **Code to bridge USB and DAP data flow** is licensed under [Apache License, Version 2.0](www.apache.org/licenses/LICENSE-2.0)
* Each source/header file specifies corresponding license/copyright notices.
