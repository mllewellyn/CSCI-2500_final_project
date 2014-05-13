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
		addi v0, zero, 11
		addi a0, zero, 10 # ascii for new line
		syscall
		addi v0, zero, 1 # reload for printing int
		nop  # not entirely sure why this has to be here, bug in the code?? without it the above instruction doesn't work
		jr ra


	print_success: 
		addi t5, t5, 1 # add 1 to t5, it's the success coutner
		addi v0, zero, 11
		addi a0, zero, 't'
		syscall
		addi a0, zero, 10 # ascii for new line
		syscall

		j ra
			.data
string1: 	.asciiz "Print this. \n"
	# main:		
					.text
	__start:		li v0, 4
					la a0, string1
					syscall
	# //Simulator loads sp with a valid address before starting program


	#jal test_rtype
	sub s0, ra, 500
	bltzal s0, test_itype
	#  jal test_itype
	#jal itype_alu

		#jr ra

	# main:
	# //Simulator loads sp with a valid address before starting program
	
	jal test_rtype
	# jal test_itype
	# jal itype_alu

	j quit


	test_itype:
	#la t1, array1
	# start by storing ra
	#addi s0, ra, 0
	#la t0, array1
	#li t1, 5
	#sw t1, t0
	#li t1, 13
	#sw t1, 4(t0)
	#li t1,-7
	#sw t1, 8(t0)
	#li t6, 1
	#li t7, 4
	#sw t8, 0(t7)
	#sw t5,4(s1)
	#li v0, 1
	#add a0, t1, 0
	#add a0, t0, zero
	#syscall
	


	# I ALU: ADDI, ADDIU, ANDI, LUI, ORI, XORIc, SLTI, SLTIU
	# I branch: BEQ, BGEZ, BGTZ, BLEZ, BLTZ, BNE, BGEZAL, BLTZAL
	# I load/store: LB, LW, SB, SW

	# jal itype_alu
	jal itype_branch

	# restore ra (don't really have to but want to)
	addi ra, s0, 0
	jr ra

	itype_alu:
	# start by storing ra
	addi s1, ra, 0
	# I ALU: ADDI, ADDIU, ANDI, LUI, ORI, XORIc, SLTI, SLTIU
	addi t0, zero, 25432
	addi v0, zero, 1
	# ADDI, ADDIU, ANDI, LUI, ORI, XORIc, SLTI, SLTIU,
	addi a0, t0, 12345
	syscall
	jal print_newline

	addi a0, t0, -12345
	syscall
	jal print_newline

	addiu a0, t0, 12345
	syscall
	jal print_newline

	addiu a0, t0, -12345 # not sure what this should do think the same thing
	syscall
	jal print_newline

	andi a0, t0, 12345
	syscall
	jal print_newline

	lui a0, 12345
	syscall
	jal print_newline

	ori a0, t0, 12345
	syscall
	jal print_newline
	
	xori a0, t0, 12345
	syscall
	jal print_newline

	addi a0, t0, 12345
	syscall
	jal print_newline

	slti a0, t0, 12345
	syscall
	jal print_newline

	slti a0, t0, -12345
	syscall
	jal print_newline

	slti a0, t0, 32000
	syscall
	jal print_newline

	slti a0, t0, -32000
	syscall
	jal print_newline

	sltiu a0, t0, 12345
	syscall
	jal print_newline

	sltiu a0, t0, -12345
	syscall
	jal print_newline

	sltiu a0, t0, 32000
	syscall
	jal print_newline

	sltiu a0, t0, -32000
	syscall
	jal print_newline

	# restore ra (don't really have to but want to)
	addi ra, s1, 0
	jr ra

	itype_branch:
	# I branch: BEQ, BGEZ, BGTZ, BLEZ, BLTZ, BNE, BGEZAL, BLTZAL
	# start by storing ra
	addi s1, ra, 0
	#first don't branch then branch, if ever do branch when shouldn't end early and vis versa
	addi t0, zero, 254
	addi t1, zero, 123
	addi t5, zero, 0

	beq t0, t1, branch_failure
	jal print_success
	addi t1, zero, 254
	beq t0, t1, itype_branch_1
	j branch_failure
	itype_branch_1:
	jal print_success

	bgez t1, itype_branch_2
	j branch_failure
	itype_branch_2:
	jal print_success
	addi t1, zero, 0
	bgez t1, itype_branch_3
	j branch_failure
	itype_branch_3:
	jal print_success
	addi t1, zero, -50
	bgez t1, branch_failure
	jal print_success

	addi t1, zero, -50
	bgtz t1, branch_failure
	jal print_success
	addi t1, zero, 254
	bgtz t1, itype_branch_4
	j branch_failure
	itype_branch_4:
	jal print_success

	addi t1, zero, -50
	blez t1, itype_branch_5
	j branch_failure
	itype_branch_5:
	jal print_success
	addi t1, zero, 0
	blez t1, itype_branch_6
	j branch_failure
	itype_branch_6:
	jal print_success
	addi t1, zero, 254
	blez t1, branch_failure
	jal print_success

	addi t1, zero, -50
	bgtz t1, branch_failure
	jal print_success
	addi t1, zero, 254
	bgtz t1, itype_branch_7
	j branch_failure
	itype_branch_7:
	jal print_success

	addi t1, zero, 254
	bltz t1, branch_failure
	jal print_success
	addi t1, zero, -52
	bltz t1, itype_branch_8
	j branch_failure
	itype_branch_8:
	jal print_success

	addi t0, zero, 254
	addi t1, zero, 254
	bne t0, t1, branch_failure
	jal print_success
	addi t1, zero, 123
	bne t0, t1, itype_branch_9
	j branch_failure
	itype_branch_9:
	jal print_success


# BGEZAL
# BLTZAL


	# print total # of successes
	addi v0, zero, 11
	addi a0, zero, 't'
	syscall
	addi a0, zero, 's'
	syscall
	addi a0, zero, '='
	syscall
	addi v0, zero, 1
	addi a0, t5, 0
	syscall
	addi v0, zero, 11
	addi a0, zero, 10 # new line
	syscall
	# restore ra (don't really have to but want to)
	addi ra, s1, 0
	jr ra

	
	test_rtype:
	# test R type alu instrucitons
	# R ALU: ADD, ADDU, AND, OR, SUB, SUBU, XOR, SLT, SLTU, SLL, SLLV,
	# 	SRA, SRL, SRLV, DIV, DIVtest_rtypeU, MULT, MULTU
	# start by storing ra, yes this is lazy it's oaky
	
	addi s0, ra, 0
	# load numbers into t0 and t1 and v0 to start

	addi t0, zero, 254
	addi t1, zero, 123

	# addi t0, zero, 254
	# addi t1, zero, 123

	addi v0, zero, 1
	# do some math and print some numbers
	# add a0, t0, t1
	# syscall
	# jal print_newline

	# addu a0, t0, t1
	# syscall
	# jal print_newline

	# and a0, t0, t1
	# syscall
	# jal print_newline

	# or a0, t0, t1
	# syscall
	# jal print_newline

	# sub a0, t0, t1
	# syscall
	# jal print_newline

	# subu a0, t0, t1
	# syscall
	# jal print_newline

	# xor a0, t0, t1
	# syscall
	# jal print_newline

	# # here's where it gets weird
	# slt a0, t0, t1
	# syscall
	# jal print_newline

	# sltu a0, t0, t1
	# syscall
	# jal print_newline

	# #also rev slt to make sure it works
	# slt a0, t1, t0
	# syscall
	# jal print_newline

	# sltu a0, t1, t0
	# syscall
	# jal print_newline

	# # have smaller shifts
	# addi t1, zero, 3
	# sll a0, t0, 3
	# syscall
	# jal print_newline

	# sllv a0, t0, t1
	# syscall
	# jal print_newline

	# sra a0, t0, 3
	# syscall
	# jal print_newline

	# srl a0, t0, 3
	# syscall
	# jal print_newline

	# srlv a0, t0, t1
	# syscall
	# jal print_newline

	# multiply and divide, should have remainders and overflow
	# load new stuff to keep it interesting
	addi t0, zero, 2543
	addi t1, zero, 0

	# mult t0, t1
	# mfhi a0
	# syscall
	# jal print_newline
	# mflo a0
	# syscall
	# jal print_newline

	# multu t0, t1
	# mfhi a0
	# syscall
	# jal print_newline
	# mflo a0
	# syscall
	# jal print_newline
	
	# this is being weird
	nop
	div t0, t1 			# no idea why this isn't being called when assembled
	nop
	mfhi a0
	syscall
	jal print_newline
	mflo a0
	syscall
	jal print_newline

	addi ra, s0, 0

	j ra
test: 
	addi a0, zero, 'A'
	syscall
	jr ra


branch_failure:
	# print an F because branching failed
	addi v0, zero, 11
	addi a0, zero, 'F'
	syscall
	j quit

quit:
	addi v0, zero, 10				# quit program
	syscall
