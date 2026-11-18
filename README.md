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
  inc/    can.h  can_types.h  can_timing.h  can_util.h
  src/    can.c  can_timing.c  can_util.c  main.c  startup_stm32f446xx.s
  tests/  unity/  testfiles/test_bit_timing.c
  CMSIS/  linker.ld  Makefile
```

- `can_timing.c` and `can_util.c` are pure C with no CMSIS include, so they build and test on the PC.
- `can.c` holds everything that touches registers. It keeps one private pointer, `s_can`, to the CAN peripheral.

## Build, test, debug

```
make test           # build the Unity host tests (PC)
make runtest        # run them
make build_can      # cross-compile can_project.elf (arm-none-eabi-gcc, -g -O0)

openocd -f interface/stlink.cfg -f target/stm32f4x.cfg     # terminal 1
make gdb                                                   # terminal 2
(gdb) target extended-remote :3333
(gdb) monitor reset halt
(gdb) load
```

## Driver status

Verified so far in loopback mode (500 kbit/s, PCLK1 = 16 MHz HSI, 87.5 % sample point), by reading registers in GDB:

| Function | File | Status |
|---|---|---|
| `can_wait_flag` (timeout helper) | can_util.c | done, Unity-tested |
| `can_calc_bit_timing` | can_timing.c | done, Unity-tested |
| `can_init` (clocks, pins AF9, reset, MCR options, BTR) | can.c | done, registers match |
| `can_filter_accept_all` | can.c | done |
| `can_start` | can.c | done |
| `can_transmit` | can.c | done, frame ID 0x123 / 5 bytes sent |
| `can_receive`, `can_rx_pending`, `can_tx_free_mailboxes` | can.c | done, frame read back with the right ID, length and data |

Milestone: a frame built in software goes out through a TX mailbox, loops back through the bit-timing logic and the filter, and comes back from RX FIFO0 unchanged.

### Lessons learned (bugs fixed on the way)

- FS1R: a **1** means a 32-bit filter bank, not 0.
- TX mailbox registers are not at the top of `CAN_TypeDef`: use `sTxMailBox[n]` for TX and `sFIFOMailBox[n]` for RX.
- Write TXRQ last, with `|=`, never `=`, or the ID is wiped.
- Return right after starting the first free mailbox, or the frame is sent three times.
- Clear TDLR and TDHR before packing bytes, because mailbox registers are undefined after reset.
- `RDTR` holds more than the length: mask the low 4 bits, then clamp to 8.
- Release the RX FIFO with a plain `=` on RFOM, never `|=`, because FULL and FOVR clear when 1 is written.
- A frame takes about 200 microseconds, so wait for `can_rx_pending` before reading it.

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