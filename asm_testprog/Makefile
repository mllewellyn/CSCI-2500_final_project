all:
	mipsel-linux-gnu-gcc asm_testprog.S -o asm_testprog.elf -nostdlib -nostartfiles -fno-delayed-branch
	find uni_pacific_test_cases/ -name "*.S" -exec mipsel-linux-gnu-gcc {} -o {}.elf -mmemcpy -nostdlib -nostartfiles -fno-delayed-branch \;