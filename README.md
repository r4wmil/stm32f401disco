# STM32F401 Discovery experimentation
## Building
```sh
make EXAMPLE=1_blink
# 1_blink: Blinking Program
# 2_conv: Convolutional Tests
# 3_pwm_leds: PWM with leds
# 4_pwm_mul: PWM with multiple leds
# 5_button: button example with debounce
# e. t. c
# ...
```

## Requirments
```txt
arm-none-eabi-gcc
arm-none-eabi-gdb / gdb-multiarch
arm-none-eabi-newlib
stlink
openocd
```

## Where got files
- `./STM32F401CCUX_FLASH.ld` - STM32CubeF4 -> ./Projects/STM32F401-Discovery/Examples/GPIO/GPIO_EXTI/STM32CubeIDE/STM32F401CCUX_FLASH.ld
- `./startup_stm32f401xc.s` - STM32CubeF4 -> ./Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc
- `./system_stm32f4xx.c` - STM32CubeF4 -> ./Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/
- `./Drivers/CMSIS/Core/Include` - STM32CubeF4 -> ./Drivers/CMSIS/Include
- `./Drivers/CMSIS/Device/ST/STM32F4xx` - STM32CubeF4 -> ./Drivers/STM32F4xx

## Specs
- F - general purpose
- 4 - fourth gen
- 0 - access line
- 1 - mainstream performance line
- V - 100 pins
- C - 256KB Flash
- T - industrial temp range (-40C, +85C)
- 6 - LQFP circuit package
- U - no internal voltage regulator

STM32 naming conventions:
- https://www.compilenrun.com/docs/iot/stm32/stm32-fundamentals/stm32-naming-convention/
CMSIS - Cortex Microcontroller Software Interface Standart

## Misc
### ARM toolchain on MSYS2
```
pacman -S mingw-w64-ucrt-x86_64-arm-none-eabi-toolchain
pacman -S mingw-w64-ucrt-x86_64-arm-none-eabi-gdb
pacman -S mingw-w64-ucrt-x86_64-stlink
```

### WSL
```
winget.exe install --interactive --exact dorssel.usbipd-win
alias usbipd='/mnt/c/Program\ Files/usbipd-win/usbipd.exe'
usbipd list
export BUSID=<BUSID>
powershell.exe -Command "Start-Process 'C:\Program Files\usbipd-win\usbipd.exe' -Verb RunAs -ArgumentList 'bind --busid $BUSID'"
powershell.exe -Command "Start-Process 'C:\Program Files\usbipd-win\usbipd.exe' -Verb RunAs -ArgumentList 'attach --wsl --busid $BUSID'"
```
