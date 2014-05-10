# /**
# 	@file
# 	@author Andrew D. Zonenberg
# 	@brief Test program
 # */

#include "registers.h"

# uncomment for mipsel-linux-gnu-gcc also remove 
.globl __start
# uncomment for qtspim, also put in freaking 
# .globl main

	print_newline: 
		# prints a new line and then puts print int back in v0 to make life easy
		li v0, 11
		li a0, 10 # ascii for new line
		syscall
		li v0, 1 # reload for printing int
		nop  # not entirely sure why this has to be here, bug in the code??
		j ra

	# main:
	__start:
	# //Simulator loads sp with a valid address before starting program

	# test R type alu instrucitons
	# R ALU: ADD, ADDU, AND, OR, SUB, SUBU, XOR, SLT, SLTU, SLL, SLLV,
	# 	SRA, SRL, SRLV, DIV, DIVU, MULT, MULTU
	# load numbers into t0 and t1 and v0 to start 
	li t0, 254
	li t1, 123
	li v0, 1
	# do some math and print some numbers
	add a0, t0, t1
	syscall
	jal print_newline

	addu a0, t0, t1
	syscall
	jal print_newline

	and a0, t0, t1
	syscall
	jal print_newline

	or a0, t0, t1
	syscall
	jal print_newline

	sub a0, t0, t1
	syscall
	jal print_newline

	subu a0, t0, t1
	syscall
	jal print_newline

	xor a0, t0, t1
	syscall
	jal print_newline

	# here's where it gets weird
	slt a0, t0, t1
	syscall
	jal print_newline

	sltu a0, t0, t1
	syscall
	jal print_newline

	#also rev slt to make sure it works
	slt a0, t1, t0
	syscall
	jal print_newline

	sltu a0, t1, t0
	syscall
	jal print_newline

	# have smaller shifts
	li t1, 3
	sll a0, t0, 3
	syscall
	jal print_newline

	sllv a0, t0, t1
	syscall
	jal print_newline

	sra a0, t0, 3
	syscall
	jal print_newline

	srl a0, t0, 3
	syscall
	jal print_newline

	srlv a0, t0, t1
	syscall
	jal print_newline

	# multiply and divide, should have remainders and overflow
	# load new stuff to keep it interesting
	li t0, 2543
	li t1, 123
	# this is being weird
	# div t0, t1
	# mfhi a0
	# syscall
	# jal print_newline
	# mflo a0
	# syscall
	# jal print_newline

	mult t0, t1
	mfhi a0
	syscall
	jal print_newline
	mflo a0
	syscall
	jal print_newline

	# multu t0, t1
	# mfhi a0
	# syscall
	# jal print_newline
	# mflo a0
	# syscall
	# jal print_newline


quit:
	li v0, 10				# quit program
	syscall
