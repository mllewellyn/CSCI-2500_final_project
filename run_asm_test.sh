echo "Making files"
make -C asm_testprog/
make -C sim/
echo ""
echo "running test"
./sim/sim.out asm_testprog/asm_testprog.elf