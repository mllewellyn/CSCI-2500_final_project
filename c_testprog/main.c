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

int main()
{
	const char* str = "hello world\n";
	do_syscall((unsigned int)str, 0, SYS_PRINT_STR);	// print string syscall
	int a = 1234;
	do_syscall((unsigned int)&a, 0, 1); 	//print int syscall
	char c = 'h';
	do_syscall((unsigned int)&c, 0, 12);	//print char syscall
	
	return 0;
}