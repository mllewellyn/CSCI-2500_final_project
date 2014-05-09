echo "Making files"
if make -C asm_testprog/ ; then
	# make -C sim/
	if make -C sim/ ; then
		echo -e "\nrunning test"
		# comment in the test(s) you want to run
		./sim/sim.out asm_testprog/asm_testprog.elf
		# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example1.S.elf
		# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example2_hello_world.S.elf
		# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example3_io.S.elf
		# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example4_loop.S.elf
		# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example5_function_without_stack.S.elf
		# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example5_function_with_stack.S.elf
		# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example6_loop_with_function.S.elf
		# ./sim/sim.out asm_testprog/max_hw3/SPIM_Calc.s.elf
	else
		echo -e "\nSIM compile failed"
	fi
else
	echo -e "\n MIPS assembly failed"
fi