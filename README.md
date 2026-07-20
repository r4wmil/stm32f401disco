# STM32F401 Discovery experimentation
## Building
```sh
git submodule update --init --recursive
gcc build.c -o build
bear -- ./build 4_hal_blink
```

## Requirments
```txt
arm-none-eabi-gcc
arm-none-eabi-gdb
arm-none-eabi-newlib
stlink
openocd
```

## Where got files

`./STM32F401CCUX_FLASH.ld` - STM32CubeF4 -> ./Projects/STM32F401-Discovery/Examples/GPIO/GPIO_EXTI/STM32CubeIDE/STM32F401CCUX_FLASH.ld
`./startup_stm32f401xc.s` - STM32CubeF4 -> ./Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc
`./system_stm32f4xx.c` - STM32CubeF4 -> ./Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/
`./CMSIS/Core/Include` - STM32CubeF4 -> ./Drivers/CMSIS/Core/Include
`./CMSIS/Include` - STM32CubeF4 -> ./Drivers/CMSIS/Core/Include

## Specs
F - general purpose
4 - fourth gen
0 - access line
1 - mainstream performance line
V - 100 pins
C - 256KB Flash
T - industrial temp range (-40C, +85C)
6 - LQFP circuit package
U - no internal voltage regulator
STM32 naming conventions:
- https://www.compilenrun.com/docs/iot/stm32/stm32-fundamentals/stm32-naming-convention/

## Misc
### ARM toolchain on MSYS2
```
pacman -S mingw-w64-ucrt-x86_64-arm-none-eabi-toolchain
pacman -S mingw-w64-ucrt-x86_64-arm-none-eabi-gdb
pacman -S mingw-w64-ucrt-x86_64-stlink
```
