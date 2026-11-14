test: tests/testfiles/test_bit_timing.c src/can.c tests/unity/src/unity.c
	gcc tests/testfiles/test_bit_timing.c src/can_timing.c tests/unity/src/unity.c -Iinc -Itests/unity/src -ICMSIS/Device/ST/STM32F4xx/Include -ICMSIS/Include -o test_bit_timing.exe

runtest: test_bit_timing.exe
	./test_bit_timing.exe	

build_can: src/main.c src/can.c src/can_timing.c src/can_util.c
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -g -O0 \
	-DSTM32F446xx -Iinc -ICMSIS/Include -ICMSIS/Device/ST/STM32F4xx/Include \
	src/main.c src/can.c src/can_timing.c src/can_util.c src/startup_stm32f446xx.s \
	-T linker.ld -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs \
	-o can_project.elf
	arm-none-eabi-size can_project.elf

# OpenOCD configuration variables
OPENOCD = openocd
OPENOCD_INTERFACE = interface/stlink.cfg
OPENOCD_TARGET = target/stm32f4x.cfg
TARGET_ELF = can_project.elf

flash: $(TARGET_ELF)
	$(OPENOCD) -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET) -c "adapter speed 500" -c "reset_config srst_only" -c "program $(TARGET_ELF) verify reset exit"

clean: *.elf *.exe
	rm *.elf *.exe
