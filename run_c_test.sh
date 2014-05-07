echo "Making files"
if make -C c_testprog/ ; then
	# make -C sim/
	if make -C sim/ ; then
		echo -e "\nrunning test"
		./sim/sim.out c_testprog/c_testprog.elf
	else
		echo -e "\nSIM compile failed"
	fi
else
	echo -e "\n C test program failed"
fi