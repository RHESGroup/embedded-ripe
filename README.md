# embedded-ripe

This repository presents a novel version of the [RIPE benchmark](https://github.com/johnwilander/RIPE), thought for Microcontroller that runs [FreeRTOS](https://www.freertos.org/).

RIPE was originally developed by John Wilander and Nick Nikiforakis and the [paper](papers/ripe_paper.pdf) was presented at The 27th Annual Computer Security Applications Conference (ACSAC) in Orlando, Florida, December 5-9, 2011.

## Preliminary
### Supported microcontrollers

Currently, the only architecture supported is the `Cortex-M4`. Currently, we aim to support the following boards:

&nbsp; | MCU          | Boards                                                                              | Board name
-------|--------------|-------------------------------------------------------------------------------------|---
**F4** | &nbsp;       | &nbsp;                                                                              | &nbsp;
&nbsp; | STM32F407VGT | [STM32F4-Discovery](https://www.st.com/en/evaluation-tools/stm32f4discovery.html)   | stm32f4-discovery
&nbsp; | STM32F429ZI  | [STM32 Nucleo-144](https://www.st.com/en/evaluation-tools/nucleo-f429zi.html)       | nucleo-144

No hardware yet? Don't worry! 
You can try to run the benchmark on a virtalized environment. We aim to support the following virtualized boards.

&nbsp; | MCU          | Boards                                                                              | Board name
-------|--------------|-------------------------------------------------------------------------------------|---
&nbsp; | STM32F405RG  | [Netduino-Plus 2](https://www.sparkfun.com/products/retired/11608)                  | netduinoplus2

To execute the benchmark on a virtualized board, you will need QEMU. 
[Here](https://www.freertos.org/install-and-start-qemu-emulator/) you can find some info about installing QEMU on your host machine.

To get the full support to `netduinoplus2` board, you must install QEMU v6 or above.

### Getting started with FreeRTOS 

This benchmark is released as a [FreeRTOS/Demo](https://www.freertos.org/a00102.html). Typically, Demos are a way to create new Applications taking advantage of FreeRTOS kernel and libraries. We ship the benchmark as a FreeRTOS/Demo to make it simpler execute it on your board.

Do you want to execute this benchmark on a different board? It should be not difficult to realize a Demo application for your board. You can start reading from this [page](https://www.freertos.org/a00090.html)

## Getting started: build and run
### Dependencies

The following setup is valid only for Linux host machines. If you are using Windows or MacOS consider we cannot guarantee you will be able to setup the environment.

The list of dependencies depends on the target board you are using, but at least you need:
- Make
- python3
- [GNU Cross-Compile toolchain for ARM](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads). Commonly, you can install the toolchain also by means of your packet manager (i.e. `apt install gcc-arm-none-eabi`)
- An editor/IDE! We provide some launch and debug actions for [Visual Studio Code](https://code.visualstudio.com/).

Target dependent or optional tools:
- A **programmer** for your board. We currently support [stlink](https://github.com/texane/stlink). [OpenOCD](http://openocd.org/) could be fine too, but it is not tested.
- **QEMU** if you intend to execute the benchmark in a virtualized environment.
- **GDB** if you want to debug your code. Note, you need the multi-architecture version or the one offered with ARM Toolchain.

## Clone

Clone this repository directly in the FreeRTOS demo folder

```
# starting from FreeRTOS root folder
cd FreeRTOS/Demo
git clone git@github.com:RHESGroup/embedded-ripe.git CORTEX_M4F_STM32_QEMU_GCC
```

### Build

Change into `build` folder and run the `make` command.

```
cd CORTEX_M4F_STM32_QEMU_GCC/build
make <target> # could be 'qemu' / 'nucleo' / 'discovery'
```

Then, to flash the binary into the board (or load it into QEMU) you can execute

```
make <target>-run
```

### Debug

Using GDB (the version provided by ARM toolchain or the multi architecture one), you can debug the benchmark step by step.

**Debug on hardware target**

To debug the code on the hardware target, connect the board to the host machine and run

```
make <target>-debug
```

The make target above launch the `st-util` binary (comes with `stlink` package) that it will connect to the board programmer and will act as a GDB-server. Then you can launch

```
gdb-multiarch --target-remote localhost:4242
```

or you the launch configuration provided if you are working with VS Code.

**Debug on QEMU**

To debug the code on QEMU, launch QEMU with

```
make qemu-debug
```

The make target above launch QEMU with the debug flag. The, you can attach your GDB session with

```
gdb-multiarch --target-remote localhost:1234
```

or you the launch configuration provided if you are working with VS Code.

## About the benchmark

### Follow-ups & Development status

Originally, in 2011, the [RIPE benchmark](https://github.com/johnwilander/RIPE) was developed for `x86` architecture. 

In 2018, Draper Labs realized a port for `risc-V` architecture, called [hope-RIPE](https://github.com/draperlaboratory/hope-RIPE)

> The RIPE port to RISC-V was developed by John Merrill. RIPE for RISC-V is designed for use with the Spike simulator.

Hubert Rosier realized a port for `x86-64`, called [RIPE64](https://github.com/hrosier/ripe64).

> This port was developed by Hubert ROSIER for an academic project in the National University of Singapore. The project was supervised by Professor Roland YAP and co-supervisor senior research fellow Gregory James DUCK of the School of Computing of the National University of Singapore.

Gabriele Serra realized improved the benchmark implementing a hardware abstraction layer and re-implementing attacks for ARM 64 architecture. The [project](https://github.com/gabriserra/RIPE) is a fork of the original one and integrates the subsequent versions.

> The HAL and the `aarch64` port was developer by Gabriele Serra a PhD student at Scuola Superiore Sant'Anna, Pisa. The project is still under development and currently there is no working version. Contributions are welcome.

This repository would not host any of the material for the previous projects. Unfortunately, to implement a benchmark for Microntroller without any assumption of their architecture features and OS running configuration requires some heavy modification of the original idea. **Hence, this repository exploit the RIPE idea but does not make use of any of the previous contributions.**


### Individual test with attack parameters

Currently, this version of supports the following attack parameters:
- **Location** can be: `stack`, `bss` or `data`
- **Target pointer** can be: `ret`, `funcptr` or `structfuncptr`
- **Target function** can be: `strcpy`, `strncpy`, `memcpy`, `memmove` or `homebrew` (a loop-based equivalent version of memcpy)
- **Attack code** can be `injected-svc`,`injected-svc-nop`, `reused-svc`

#### Location

The overflow location describes the memory section in which the target 
buffer is located.

#### Target Code Pointer

The target code pointer represents the pointer exploited by the specific attack. It transfers the control flow to the appropriate offset to trigger the payload. 
#### Target function

Em-RIPE uses 5 vulnerable functions as attack entry points. Such functions from allow to copy part of a string from a source buffer to a destination buffer. 
#### Attack code

The attack code represents the payload that an intruder may want to get running through a buffer overflow exploit.
## License & Citation

RIPE was originally released under the [MIT licence](https://choosealicense.com/licenses/mit/). 

Subsequent modification and porting to other architectures were still released under the MIT license.

If you use, extend or build upon RIPE we kindly ask you to cite the original ACSAC paper in which RIPE is presented. Here's the BibTeX:

```
@inproceedings{ wilander.ripe,
    author = {John Wilander and Nick Nikiforakis and Yves Younan and Mariam Kamkar and Wouter Joosen},
    title = {{RIPE}: Runtime Intrusion Prevention Evaluator},
    booktitle = {In Proceedings of the 27th Annual Computer Security Applications Conference, {ACSAC}},
    year = {2011},
    publisher = {ACM}
}
```