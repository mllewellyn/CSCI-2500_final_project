# SPIM_Cacl
# Max Llewellyn
# CSCI-2500 HW 3

	# .data

# welcome:.asciiz "Welcome to SPIM Calculator 1.0!"
# prompt1:.asciiz "\n\nEnter the first number: "
# prompt2:.asciiz "Enter the second number: "
# opPrompt:.asciiz "Enter the operation (+, -, *, /), then press enter key: "
# equals: .asciiz " = "
# againPrompt: .asciiz "\n\nAnother Calculation (y, n)? "
# errorMessage: .asciiz "\nInvalid Operator!"
# newline: .asciiz "\n"
# space: .asciiz " "
# leftParen: .asciiz " ("
# rightParen: .asciiz ")"
# bye: .asciiz "\n\nCalculator Terminated\n"

	.globl	__start
	
	.text
__start: # the main procedure

	jal printWelcome		# call the printWelcome procedure
startOfCalc:
	jal getUserInputInts	# call the getUserInput procedure
	# store the results of get user input
	move $s0, $v0	# store the first int
	move $s1, $v1	# store the second int
	jal getUserInputOpereation # call the getUserInputOpereation procedure
	move $s2, $v0	# store the operation character
	# set up args for choose operation procedure
	move $a0, $s0	# first int
	move $a1, $s1	# second int
	move $a2, $s2	# operation char
	jal chooseOpereation 	# go to the choose operation section of the code
	# store the results of the operation
	move $s3, $v0
	move $s4, $v1
	# set up the args for the printEquation subroutine
	move $a0, $s0
	move $a1, $s1
	move $s2, $s2
	jal printEquation			# call the printEquation subroutine
	# set up the args for the print result subroutine
	move $a0, $s3
	move $a1, $s2
	move $a2, $s4
	jal printResult				# call the printResult subroutine
	jal promptAgain		# call the promptAgain subroutine, this will exit the program if the user so desires
	j startOfCalc		# jump back to the top


# procedure to print the welcome message
printWelcome:
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, welcome 	# load address, load the address of the welcome string into $a0 so syscall will print it
	syscall				# print the welcome string
	j $ra 				# jump back to where the procedure was called

# procedure to prompt and record user input
getUserInputInts: 

	# prompt user for first value
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, prompt1 	# load address, load the address of the welcome string into $a0 so syscall will print it
	syscall				# print the prompt1 string

	# read the first integer from the user
	li	$v0, 5			# load immediate, store 5 in the $v0 register so syscall will load an int
	syscall				# load the integer
	# put the integer on the stack to store it
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $v0, 0($sp) 		# put the int on the stack

	# prompt user for second  value
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, prompt2 	# load address, load the address of the welcome string into $a0 so syscall will print it
	syscall				# print the prompt1 string

	# read the second integer from the user
	li	$v0, 5			# load immediate, store 5 in the $v0 register so syscall will load an int
	syscall				# load the integer
	move $v1, $v0		# more directly to return register
	#get the first int from the stack
	lw $v0, 0($sp)		# copy the int from the stack
	addi $sp, $sp, 4 	# increment the stack pointer

	j $ra 				# return

# prompt user for operation char and record it
getUserInputOpereation:

	# prompt the user for an operation 
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, opPrompt 	# load address, load the address of the welcome string into $a0 so syscall will print it
	syscall				# print the prompt1 string

	# read the operation char from the user
	li	$v0, 12			# load immediate, store 12 in the $v0 register so syscall will load a char
	syscall				# load the char
	# move $v0, $v0		# move the char to the return register, don't need to do this because they're the same

	j $ra # return

chooseOpereation:

	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $ra, 0($sp) 		# put the return address on the stack

	# don't need to set up the arguments for the math operations, 
	# they're the same as the arguments for this subroutine
	# move $v0, $v0 	# the result of the math
	# move $v1, $v1 	# the remainder if relevant

	# now time to determine which char it is
	# showing multiple ways to compare to a constant char
	addi $t0, $zero, 43		# the ascii code for the plus sign 43
	bne $a2, $t0, chooseOpereationL1	# check if it's a plus sign, if so continue to addition, otherwise branch to next test
	jal addition 			# call the addition subroutine 
	j finishChooseOpereation		# jump to store finishChooseOpereation
chooseOpereationL1:
 	bne $a2, '-', chooseOpereationL2
 	jal subtract 	# check if it's a minus sign, if so branch to subtract
 	j finishChooseOpereation
chooseOpereationL2:
	bne $a2, '*', chooseOpereationL3
	jal multiply 	# check if it's a star, if so branch to multiply
	j finishChooseOpereation
chooseOpereationL3:
	bne $a2, '/', error 	#if not a slash there must be an error
	jal divide 	# check if it's a slash, if so branch to divide
	j finishChooseOpereation

# a label in the chooseOpereation subroutine
finishChooseOpereation:
	# put the result of the math function where it belongs
	# don't need to do this because they're the same
	# move $v0, $v0 	# the result of the math
	# move $v1, $v1 	# the remainder if relevant
	
	lw $ra, 0($sp)		# load the original return address
	addi $sp, $sp, -4	# clean up the stack
	j $ra 				# return


addition: # add the two integers together and jump to printing results

	add $v0, $a0, $a1 	# add the two integers together
	j $ra			# return

subtract: # subtract the two integers and jump to printing results

	sub $v0, $a0, $a1 	# subtract from the two integers
	j $ra				# return

determineSign: # procedure to determine the sign of multiplying or dividing the two variables
	# returns 0x00000000 if result will be positive and 0xFFFFFFFF if result should be negative
	sra $t0, $a0, 31	# fill $t0 with the sign bit of $a0
	sra $t1, $a1, 31	# fill $t1 with the sign bit of $a1
	xor $v0,$t0,$t1     # xor the sign bits and store the result in $v0
	j $ra 	# return

absValue: # return the abs value of $a0 in $v0
	# sra $v0, $a0, 31	# fill $t0 with the sign bit of $a0
	# xor $v0,$v0,$a0		# if sign bit is 0 all bits unchanged, otherwise all bits inverted
	# sub $v0, $v0, $t1	# either -1 or 0 is subtracted from result of xor to square everything up
	move $v0, $a0
	j $ra 	# return

multiply: # multiply the two integers by using an addition loop and jump to printing the results
	
	# mul $v0, $a0, $a1  #this is multiplication the easy way
	# this is multiplication the hard way
	# put the args on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $ra, 0($sp) 		# put the return address on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $a0, 0($sp) 		# put the first int on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $a1, 0($sp) 		# put the second int on the stack
	# determine the sign bit
	jal determineSign
	# put the result on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $v0, 0($sp) 		# put the result of the determine sign on the stack
	# take the abs of both args
	lw $a0, 8($sp)		# load the first int from the stack
	jal absValue		# call absValue subroutine
	# put the result on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $v0, 0($sp) 		# put the int on the stack
	lw $a0, 8($sp)		# load the second int from the stack
	jal absValue		# call absValue subroutine
	move $t1, $v0		# store the abs 2nd int
	lw $t0, 0($sp)		# load the abs 1st int from the stack
	addi $sp, $sp, 4 	# clean up the abs 1st int on the stack

	move $t2, $zero		# initialize the result register

multiplyLoopStart:
	beq $t1, $zero, multiplyEnd # when the count var== zero end

	add $t2, $t2, $t0 	# add a value of $t0 to the result
	addi $t1, $t1, -1  	# decrement the count
	j multiplyLoopStart # jump to the start of the loop

multiplyEnd: # end of the multiply function
	lw $t0, 0($sp) 	# load the result of the abs value from the stack
	xor $t2, $t2, $t0 	# xor with the result of the abs value 
	sub $t2, $t2, $t0 	# sub the result of the abs value to fix off by one
	move $v0, $t2 		# store correctly signed result in result register
	lw $ra, 12($sp) 	# load the return address from the stack
	addi $sp, $sp, 16	# clean up the stack
	j $ra 		# return

divide:

	# div $v0, $a0, $a1 # this is division the easy way
	# this is division the hard way
	# put the args on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $ra, 0($sp) 		# put the return address on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $a0, 0($sp) 		# put the first int on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $a1, 0($sp) 		# put the second int on the stack
	# determine the sign bit
	jal determineSign
	# put the result on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $v0, 0($sp) 		# put the result of the determine sign on the stack
	# take the abs of both args
	lw $a0, 8($sp)		# load the first int from the stack
	jal absValue		# call absValue subroutine
	# put the result on the stack
	addi $sp, $sp, -4 	# decrement the stack pointer by 4
	sw $v0, 0($sp) 		# put the int on the stack
	lw $a0, 8($sp)		# load the second int from the stack
	jal absValue		# call absValue subroutine
	move $t1, $v0		# store the abs 2nd int
	lw $t0, 0($sp)		# load the abs 1st int from the stack
	addi $sp, $sp, 4 	# clean up the abs 1st int on the stack

	move $t2, $zero		# initialize the result register

divideLoopStart:
	ble $t0, $t1, divideEnd # when $t1 is less than or equal to t2 break

	sub $t0, $t0, $t1 	# sub a value of $t1 from $t0
	addi $t2, $t2, 1  	# increment the result
	j divideLoopStart # jump to the start of the loop

divideEnd: # end of the multiply function
	move $v1, $t0 		# store what's left in $t0 as the remainder
	lw $t0, 0($sp) 		# load the result of the abs value from the stack
	xor $t2, $t2, $t0 	# xor with the result of the abs value 
	sub $t2, $t2, $t0 	# sub the result of the abs value to fix off by one
	move $v0, $t2 		# store correctly signed result in result register
	lw $ra, 12($sp) 	# load the return address from the stack
	addi $sp, $sp, 16	# clean up the stack
	j $ra 		# return

printEquation:
	# move the arguments to print to temp registers, okay because only calling syscall
	move $t0, $a0	# first int
	move $t1, $a1	# second int
	move $t2, $a2	# operation char
	
	# print a new line
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, newline 		# load address, load the address of the newline string into $a0 so syscall will print it
	syscall				# print the newline string

	# print the first number
	li $v0, 1 		# store 1 in $v0 so syscall prints an int
	move $a0, $t0	# move the first number to $a0
	syscall 		# print the first number

	# print a space
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, space 		# load address, load the address of the space string into $a0 so syscall will print it
	syscall				# print the space string

	# print the math symbol
	li	$v0, 11			# load immediate, store 11 in the $v0 register so syscall will print a char
	move $a0, $t2 	# move the code of the char to $a0 so syscall will print it
	syscall				# print the math symbol

	# print a space
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, space 		# load address, load the address of the space string into $a0 so syscall will print it
	syscall				# print the space string

	# print the second number
	li $v0, 1 		# store 1 in $v0 so syscall prints an int
	move $a0, $t1	# move the second number to $a0
	syscall 		# print the second number

	# print " = "
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, equals 	# load address, load the address of the " = " string into $a0 so syscall will print it
	syscall				# print the " = " string

	j $ra 	#return

printResult:

	# move the arguments to temp registers
	move $t0, $a0
	move $t1, $a1
	move $t2, $a2

	# print the result number
	li $v0, 1 		# store 1 in $v0 so syscall prints an int
	move $a0, $t0	# move the result number to $a0
	syscall 		# print the result number

	# if not divide go to prompt, else print remainder
	bne $t1, '/', printResultReturn 		#  if the symbol was division continue else branch to return

	# print a leftParen
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, leftParen 	# load address, load the address of the leftParen string into $a0 so syscall will print it
	syscall				# print the leftParen string

	# calc + print remainder
	li $v0, 1 		# store 1 in $v0 so syscall prints an int
	move $a0, $t2	# move the remainder number to $a0
	syscall 		# print the remainder number

	# print a rightParen
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, rightParen # load address, load the address of the rightParen string into $a0 so syscall will print it
	syscall				# print the rightParen string

printResultReturn:
	j $ra 	#return

promptAgain:

	# prompt user for another calculation
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, againPrompt # load address, load the address of the welcome string into $a0 so syscall will print it
	syscall				# print the againPrompt string
	
	# read the answer char from the user
	li	$v0, 12			# load immediate, store 12 in the $v0 register so syscall will load a char
	syscall				# load the char
	move $t0, $v0		# move the char read by syscall to a different register to store it

	bne $t0, 'y', exit 	# check if it's a 'y', if not exit else return
	j $ra 	# return

error:
	# print an error message
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, errorMessage 	# load address, load the address of the error message string into $a0 so syscall will print it
	syscall				# print the error message string
	j promptAgain		# jump to prompt again

exit:
	# say goodbye
	li	$v0, 4			# load immediate, store 4 in the $v0 register so syscall will print a string
	la	$a0, bye		# load address, load the address of the bye string into $a0 so syscall will print it
	syscall				# print the bye string

	# exit nicely
	li $v0, 10 	# load 10 into $v0 so syscall will exit
	syscall		# exit

welcome:.asciiz "Welcome to SPIM Calculator 1.0!"
prompt1:.asciiz "\n\nEnter the first number: "
prompt2:.asciiz "Enter the second number: "
opPrompt:.asciiz "Enter the operation (+, -, *, /), then press enter key: "
equals: .asciiz " = "
againPrompt: .asciiz "\n\nAnother Calculation (y, n)? "
errorMessage: .asciiz "\nInvalid Operator!"
newline: .asciiz "\n"
space: .asciiz " "
leftParen: .asciiz " ("
rightParen: .asciiz ")"
bye: .asciiz "\n\nCalculator Terminated\n"
