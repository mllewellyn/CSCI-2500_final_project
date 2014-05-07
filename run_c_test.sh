echo "making files"
make -C c_testprog/
make -C asm_testprog/
make -C sim/
echo ""
echo "running test"
./sim/sim.out c_testprog/c_testprog.elf