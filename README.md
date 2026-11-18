# FPGA-Assisted CAN Security for EV Networks

A bare-metal CAN driver for the STM32F446RE (bxCAN, CMSIS only, no HAL), built as the base for a CAN-bus security project. The plan is to build and measure the security system on STM32 alone first, then add an FPGA and compare both.

## Hardware

| Part | Role |
|---|---|
| STM32F446RE | CAN node and monitor, running the driver in this repo |
| Arduino Uno + MCP2515 module (8 MHz crystal) | second CAN node, used as the independent reference |
| 3.3 V CAN transceiver module (120 ohm termination option) | connects the STM32 CAN pins to the bus |

## Repository layout

```
STM/
  Arduino/    Folder for separate source code of ARDUINO UNO
  STM/        Folder for separate source code of STM32F446RE 
  README.md   Readme file
```

- `can_timing.c` and `can_util.c` are pure C with no CMSIS include, so they build and test on the PC.
- `can.c` holds everything that touches registers. It keeps one private pointer, `s_can`, to the CAN peripheral.

## Build, test, debug

```
Navigate in separate folders ( Arduino & STM ) to find their Makefiles & folllow the commands that's there.
```

## Driver status
Woring CAN bus : Arduino UNO sender & STM32 receiver.
Verified so far in loopback mode (500 kbit/s, PCLK1 = 16 MHz HSI, 87.5 % sample point), by reading registers in GDB:

## Bigger plan

**Phase 1: loopback driver. Done.**

**Phase 2: real bus with the MCP2515 node. Done.**
- [x] Switch to normal mode, 500 kbit/s, 62.5 % sample point to match the MCP2515 setting (CNF = 00 90 02)
- [x] Wire both nodes: CANH/CANL, shared ground, 120 ohm at both ends
- [x] Uno sends, STM32 receives and prints over UART and vice-versa
- [x] UNO sends and prints, STM32 receives and prints

**Phase 3: make the driver robust**
- [ ] Interrupt-driven RX into a software ring buffer, with overrun counting
- [ ] TX queue refilled from the TX interrupt, plus abort and status
- [ ] Error handling: TEC/REC, last error code, error-passive and bus-off recovery
- [ ] General `can_filter_config` (mask and list, 16 and 32 bit)
- [ ] Statistics (frames, drops, errors) and a register dump for debugging

**Phase 4: security node on STM32 only (baseline)**
- [ ] Silent-mode monitor with an accept-all filter, so it never disturbs the bus
- [ ] Timestamps per frame (hardware TTCM or a timer capture)
- [ ] Per-ID statistics and timing checks (rate, period, unexpected IDs)
- [ ] Attack-injection node (flooding, spoofing, bus-off attempts) to generate test cases
- [ ] Record detection results as the baseline numbers

**Phase 5: add the FPGA**
- [ ] Decide the STM32 vs FPGA work split (bit-level and error-frame visibility are what bxCAN cannot show)
- [ ] Add the FPGA path, repeat the same attacks
- [ ] Compare detection numbers against the STM32-only baseline

**Phase 6: write-up**
- [ ] Results, block diagram, and lessons learned