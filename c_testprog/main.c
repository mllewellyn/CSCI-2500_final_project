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
	const char* str1 = "\n i = ";
	const char* str2 = "\n i/3 = ";
	const char* str3 = "\n i%3 =";
	
	int i;
	for(i=0; i<5; ++i) {
		do_syscall((unsigned int)str1, 0, SYS_PRINT_STR);	// print string syscall
		do_syscall((unsigned int)i, 0, 1); 	//print int syscall
		do_syscall((unsigned int)str2, 0, SYS_PRINT_STR);	// print string syscall
		do_syscall((unsigned int)i/3, 0, 1); 	//print int syscall
		do_syscall((unsigned int)str3, 0, SYS_PRINT_STR);	// print string syscall
		do_syscall((unsigned int)i%3, 0, 1); 	//print int syscall
	}
	
	// char c = 'h';
	// do_syscall((unsigned int)c, 0, 11);	//print char syscall
	
	return 0;
}