CFLAGS=-nostdlib -nostartfiles -fno-delayed-branch -Werror -nodefaultlibs -mno-mips16 -O -mips1 -mno-abicalls -fno-pic -mno-check-zero-division
all:
	mipsel-linux-gnu-gcc -c startup.S -o startup.o $(CFLAGS)
	mipsel-linux-gnu-gcc -c main.c -o main.o $(CFLAGS)
	mipsel-linux-gnu-gcc main.o startup.o -o c_testprog.elf $(CFLAGS)
