all:
	# mipsel-linux-gnu-gcc asm_testprog.S -o asm_testprog.elf -nostdlib -nostartfiles -fno-delayed-branch
	# with all tags c code has
	# mipsel-linux-gnu-gcc asm_testprog.S -o asm_testprog.elf -nostdlib -nostartfiles -fno-delayed-branch -nodefaultlibs -mno-mips16 -O -mips1 -mno-abicalls -fno-pic 
	# with just the ones it needs to work
	mipsel-linux-gnu-gcc asm_testprog.S -o asm_testprog.elf -nostdlib -nostartfiles -fno-delayed-branch -mno-abicalls 
	# find uni_pacific_test_cases/ -name "*.S" -exec mipsel-linux-gnu-gcc {} -o {}.elf -mmemcpy -nostdlib -nostartfiles -fno-delayed-branch \;
	# find max_hw3/ -name "*.S" -exec mipsel-linux-gnu-gcc {} -o {}.elf -mmemcpy -nostdlib -nostartfiles -fno-delayed-branch \;