/**
	@file
	@author Andrew D. Zonenberg
	@brief Main program include file
 */

#ifndef sim_h
#define sim_h

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System headers

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/elf.h>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Opcode table

enum opcodes
{
	OP_RTYPE	= 0x00,
	OP_JAL		= 0x03,
	OP_ADDIU	= 0x09,
	OP_LUI		= 0x0f,
	OP_LW		= 0x23,
	OP_SW		= 0x2b
};

enum functions
{
	FUNC_JR			= 0x08,
	FUNC_SYSCALL	= 0x0c,
	FUNC_ADDU		= 0x21,
	FUNC_OR			= 0x25
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Magic register IDS

enum regids
{
	REGID_ZERO = 0,
	REGID_A0 = 4,
	REGID_SP = 29,
	REGID_RA = 31
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A MIPS instruction

//This union allows easily pulling bitfields out from instructions
union mips_instruction
{
	//R-type format
	struct
	{
		unsigned int func:6;
		unsigned int shamt:5;
		unsigned int rd:5;
		unsigned int rt:5;
		unsigned int rs:5;
		unsigned int opcode:6;
	} rtype;
	
	//I-type format
	struct
	{
		unsigned int imm:16;
		unsigned int rt:5;
		unsigned int rs:5;
		unsigned int opcode:6;
	} itype;
	
	//J-type format
	struct
	{
		unsigned int addr:26;
		unsigned int opcode:6;
	} jtype;
	
	//Write to this to load an instruction as a 32-bit word
	uint32_t word;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Virtual memory

/**
	@brief One contiguous region of virtual memory (corresponds to an ELF program header).
 */
struct virtual_mem_region
{
	uint32_t vaddr;
	uint32_t len;
	uint32_t* data;
	struct virtual_mem_region* next;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPU context

/**
	@brief All CPU registers
 */
struct context
{
	uint32_t pc;
	uint32_t hi;
	uint32_t lo;
	uint32_t regs[32];
};

enum mips_regids
{
	//because I'm gonna forget and get confused, they are automatically assigned values
	zero,
	at,
	v0, v1,
	a0, a1, a2, a3,
	t0, t1, t2, t3, t4, t5, t6, t7,
	s0, s1, s2, s3, s4, s5, s6, s7,
	t8, t9,
	k0, k1,
	gp,
	sp,
	fp,
	ra
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Startup

void ReadELF(const char* fname, struct virtual_mem_region** memory, struct context* ctx);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Simulator core

struct logging_counters
{
	// instruction  counts
	uint32_t rtypes;
	uint32_t itypes;
	uint32_t jtypes;
	// time vars
	clock_t in_time; // most recent clock in
	clock_t out_time; // most recent clock out
	// struct timespec in_time; // most recent clock in
	// struct timespec out_time; // most recent clock out1
	uint64_t elapsed_time;
};

void RunSimulator(struct virtual_mem_region* memory, struct context* ctx);

uint32_t FetchWordFromVirtualMemory(uint32_t address, struct virtual_mem_region* memory);
void StoreWordToVirtualMemory(uint32_t address, uint32_t value, struct virtual_mem_region* memory);

int SimulateInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx, 
	struct logging_counters* counters, FILE** debug_log_file);
int SimulateRtypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx);
int SimulateItypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx);
int SimulateJtypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx);
int SimulateSyscall(struct virtual_mem_region* memory, struct context* ctx, struct logging_counters* counters, FILE** debug_log_file);

//debug functions
int determineInstType(union mips_instruction* inst);
void printInstBits(union mips_instruction* inst, struct logging_counters* counters, FILE** debug_log_file);
void printInstHex(union mips_instruction* inst, struct logging_counters* counters, FILE** debug_log_file);
int32_t signFill(int32_t val, int orig_bits);

//logging functions
void print_log(struct logging_counters* counters, FILE** log_file);
void clock_in(struct logging_counters* counters);
void clock_out(struct logging_counters* counters);

#endif
