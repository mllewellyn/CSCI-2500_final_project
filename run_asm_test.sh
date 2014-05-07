echo "Making files"
make -C asm_testprog/
# this is the easy way to compile all other test cases
find asm_testprog/uni_pacific_test_cases/ -name "*.S" -exec mipsel-linux-gnu-gcc {} -o {}.elf -nostdlib -nostartfiles -fno-delayed-branch \;
make -C sim/
echo ""
echo "running test"
# comment in the test(s) you want to run
./sim/sim.out asm_testprog/asm_testprog.elf
# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example1.S.elf
# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example2_hello_world.S.elf
# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example3_io.S.elf
# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example4_loop.S.elf
# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example5_function_without_stack.S.elf
# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example5_function_with_stack.S.elf
# ./sim/sim.out asm_testprog/uni_pacific_test_cases/example6_loop_with_function.S.elf
