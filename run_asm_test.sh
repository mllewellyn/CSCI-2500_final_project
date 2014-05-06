echo "Making files"
make -C asm_testprog/
make -C sim/
echo ""
echo "running test"
./sim/sim asm_testprog/asm_testprog.elf