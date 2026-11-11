# Arduino Uno: Bare-Metal AVR Development (No Arduino IDE)

Bare-metal firmware for the Arduino Uno (ATmega328P), built and flashed entirely from the terminal in VS Code. There are no Arduino libraries, no `Serial`, no `digitalWrite()` and no Arduino IDE. Everything is plain C that talks to the hardware registers directly.

## Purpose

This folder is the **Arduino node** of my two-node CAN bus learning project, alongside an STM32F446RE node. The goals are:

- Understand what actually happens under the Arduino framework by working at the register level, with the ATmega328P datasheet open.
- Build my own drivers from scratch (GPIO, UART, timers, SPI) instead of relying on libraries.
- Have a reproducible, IDE-free toolchain (compiler, `make`, flasher) that I fully control.
- Use this base to drive an MCP2515 SPI CAN controller and exchange CAN frames with the STM32 node.

## Toolchain

| Tool | Purpose |
|------|---------|
| `avr-gcc` (+ avr-libc) | Compiler for the ATmega328P |
| `avr-objcopy`, `avr-size` | ELF to HEX conversion, memory usage report |
| `avrdude` | Flashing over USB through the Uno's stock bootloader |
| `make` | Build automation |
| VS Code (C/C++ extension) | Editor and IntelliSense |

Installed on Windows via `winget`:

```powershell
winget install --id ZakKemble.avr-gcc --exact
winget install --id AVRDudes.AVRDUDE --exact
winget install --id ezwinports.make --exact
```

Verify with `avr-gcc --version`, `avrdude -?` and `make --version`. If a command isn't found, restart VS Code so it picks up the updated `PATH`.

## Project Structure

```
Arduino/
├── inc/            # Header files (register-level driver APIs)
├── src/            # Driver and application source files
├── main.c          # Entry point
├── Makefile        # Build and flash targets
└── avrdude.conf    # avrdude configuration
```

## Build and Flash

```powershell
make          # compile and generate main.elf / main.hex
make flash    # upload to the board (set PORT in the Makefile, e.g. COM3)
make clean    # remove build artifacts
```

Equivalent manual commands:

```powershell
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -o main.elf main.c
avr-objcopy -O ihex -R .eeprom main.elf main.hex
avrdude -c arduino -p m328p -P COM3 -b 115200 -U flash:w:main.hex:i
```

To find the COM port: `[System.IO.Ports.SerialPort]::GetPortNames()`

## Progress

- [x] Installed `avr-gcc`, `avrdude` and `make` from the terminal
- [x] Fixed PATH so all tools are available in the VS Code terminal
- [x] Set up the VS Code project structure (`inc/`, `src/`, `Makefile`)
- [x] Build pipeline working: `main.c` compiles to `main.elf` and `main.hex`
- [x] Flash and verify a register-level LED blink on the board
- [x] Bare-metal UART driver (no `Serial`)
- [ ] Timer-based delays and interrupts
- [ ] SPI driver
- [ ] MCP2515 CAN controller driver
- [ ] Two-node CAN communication with the STM32F446RE

## Hardware

- Arduino Uno (ATmega328P @ 16 MHz)
- Planned: MCP2515 SPI CAN module and a CAN transceiver module for the CAN link

## References

- [ATmega328P datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)
- [AVRDUDE](https://github.com/avrdudes/avrdude)
- [avr-libc documentation](https://avrdudes.github.io/avr-libc/)