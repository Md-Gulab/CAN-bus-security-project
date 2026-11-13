test: tests/testfiles/test_bit_timing.c src/can.c tests/unity/src/unity.c
	gcc tests/testfiles/test_bit_timing.c src/can.c tests/unity/src/unity.c -Iinc -Itests/unity/src -ICMSIS/Device/ST/STM32F4xx/Include -ICMSIS/Include -o test_bit_timing.exe

runtest: test_bit_timing.exe
	./test_bit_timing.exe	