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

int main()
{
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
	
	return 0;
}