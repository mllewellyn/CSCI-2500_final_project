/**
	@file
	@author Andrew D. Zonenberg
	@brief Test program
 */

//no includes, we don't have a C standard library

//prototype for syscall wrapper in assembly code
extern unsigned int do_syscall(unsigned int a0, unsigned int a1, unsigned int syscall_num);

enum syscalls
{
	SYS_PRINT_STR = 4
};

int r_tests( unsigned int a, unsigned int b){
	b=90000000;
	a=a+b;
	do_syscall((unsigned int)a, 0, 1); 	//print int syscall
	return 0;
}

void div_loop_test() {
	const char* str1 = "\n    i= ";
	const char* str2 = "\n  i/3= ";
	const char* str3 = "\n  i3= ";
	const char* str4 = "\n i/-3= ";
	const char* str5 = "\n i-3= ";
	
	int i;
	for(i=0; i<5; ++i) {
		//do some basic div
		do_syscall((unsigned int) str1, 0, SYS_PRINT_STR);	// print string syscall
		do_syscall((unsigned int) i, 0, 1); 	//print int syscall
		do_syscall((unsigned int) str2, 0, SYS_PRINT_STR);	// print string syscall
		do_syscall((unsigned int) i/3, 0, 1); 	//print int syscall
		do_syscall((unsigned int) str3, 0, SYS_PRINT_STR);	// print string syscall
		do_syscall((unsigned int) i%3, 0, 1); 	//print int syscall
		//do some signed div
		do_syscall((unsigned int) str4, 0, SYS_PRINT_STR);	// print string syscall
		do_syscall((unsigned int) (i/-3), 0, 1); 	//print int syscall
		do_syscall((unsigned int) str5, 0, SYS_PRINT_STR);	// print string syscall
		do_syscall((unsigned int) (i%-3), 0, 1); 	//print int syscall
		do_syscall(10, 0, 11); //print a new line
	}
}

void crap_that_used_to_be_in_main() {
	const char* str = "hello world\n";
	do_syscall((unsigned int)str, 0, SYS_PRINT_STR);	// print string syscall
	unsigned int a = 1234;
	unsigned int b=23;
	a=a+1;
	a=a+b;
	a=a-5000;
	int d=200;
	a=a-d;
	a=64;
	//a=a>>2;
	b=3;
	a=a<<b;
	a=10;
	r_tests(a,b);
	//do_syscall((unsigned int)a, 0, 1); 	//print int syscall
	char c = 'h';
	do_syscall((unsigned int)c, 0, 11);	//print char syscall
}

void input_tests() {
	// not gonna read a string because that requires allocing memmory which is NOT gonna happen
	// read and print char
	char c;
	c = do_syscall(0, 0, 12); // do_syscall returns v0
	// and print it
	do_syscall((unsigned int) c, 0, 11);
	// read and print char
	int a;
	a = do_syscall(0, 0, 5); // do_syscall returns v0
	// and print it
	do_syscall((unsigned int) a, 0, 1);

}

int main()
{
	// crap_that_used_to_be_in_main();
	// div_loop_test();
	input_tests();
	
	return 0;
}