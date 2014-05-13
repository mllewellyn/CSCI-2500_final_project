Love note to TA:
	Most code is tested with the test code included, however to get div working we had to include the -mno-check-zero-division flag when compiling.
	When the compiler generated the check for 0 branching code it would only continue on to the div functino if the code was trying to div by 0,
	so that didn't make any sense. We added a check for div by 0 in our code to keep things stable.
	If you're at all interested in the git history of the project here's the github page https://github.com/mllewellyn/CSCI-2500_final_project

	Be sure to checkout sim_debug_log.txt for a list of all commands and syscalls.

	-Max and Andrew

Directory structure
	asm_testprog		Assembly-only test program (produces asm_testprog.elf). Modify as needed to test your simulator, but will not be graded.
	c_testprog			C+assembly test program (produces c_testprog.elf). Modify as needed to test your simulator, but will not be graded
	sim					The simulator itself, you will need to fill in the missing parts.

Basic workflow
	Make changes to test code in c_testprog/ or asm_testprog/ as needed
	Make changes to simulator code in sim as needed
	Run "make" in the appropriate test program directory to compile your test code to a MIPS executable.
	Go to the sim directory and run "make" to compile your simulator
	Run "./sim ../test_program_dir/input_file.elf" (using the appropriate input file name) to launch the simulation.

Basic workflow max version
	run ./rum_asm_test.sh or ./run_c_test.sh these will make the .elf, the sim and run the sim with the given .elf
