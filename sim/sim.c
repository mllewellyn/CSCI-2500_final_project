/**
	@file
	@author Andrew D. Zonenberg
	@brief The core of the simulator
 */
#include "sim.h"

/**
	@brief Read logic for instruction fetch and load instructions
	
	Address must be aligned
 */
uint32_t FetchWordFromVirtualMemory(uint32_t address, struct virtual_mem_region* memory)
{
	//Traverse the linked list until we find the range of interest
	while(memory != NULL)
	{
		//Not in range? Try next one
		if( (address < memory->vaddr) || (address >= (memory->vaddr + memory->len)) )
		{
			memory = memory->next;
			continue;
		}
		
		//Align check
		uint32_t offset = address - memory->vaddr;
		if(offset & 3)
		{
			printf("SEGFAULT: address %08x is not aligned\n", address);
			exit(1);	
		}
		
		return memory->data[offset/4];
	}
	
	//Didn't find anything! Give up
	printf("SEGFAULT: attempted to read word from nonexistent virtual address %08x\n", address);
	exit(1);
}

/**
	@brief Write logic for store instructions.
	
	Stores an entire 32-bit word. sh/sb instructions will need to do a read-modify-write structure
 */
void StoreWordToVirtualMemory(uint32_t address, uint32_t value, struct virtual_mem_region* memory)
{
	//Traverse the linked list until we find the range of interest
	while(memory != NULL)
	{
		//Not in range? Try next one
		if( (address < memory->vaddr) || (address >= (memory->vaddr + memory->len)) )
		{
			memory = memory->next;
			continue;
		}
		
		//Align check
		uint32_t offset = address - memory->vaddr;
		if(offset & 3)
		{
			printf("SEGFAULT: address %08x is not aligned\n", address);
			exit(1);	
		}
		
		memory->data[offset/4] = value;
		return;
	}
	
	//Didn't find anything! Give up
	printf("SEGFAULT: attempted to write word to nonexistent virtual address %08x\n", address);
	exit(1);
}

/**
	@brief Runs the actual simulation
 */
void RunSimulator(struct virtual_mem_region* memory, struct context* ctx)
{
	printf("Starting simulation...\n");
	
	union mips_instruction inst;
	while(1)
	{
		inst.word = FetchWordFromVirtualMemory(ctx->pc, memory);
		if(!SimulateInstruction(&inst, memory, ctx))
			break;		
	}
}

void printInstBits(union mips_instruction* inst) {
	uint32_t inst_word = inst->word;
	int opcode = inst->rtype.opcode;
	printf("DEBUG inst: ");
	int i;
	uint32_t reversed = 0;
	//because I'm really freaking lazy
	for(i=0; i<sizeof(inst_word)*8; ++i) {
		reversed += (inst_word%2)<<(sizeof(inst_word)*8-i-1);
		inst_word = inst_word>>1;
	}

	for(i=0; i<sizeof(reversed)*8; ++i) {
		printf("%d", reversed%2);
		reversed = reversed>>1;
		//add spacing so it's easier to read, first check for R type
		if(inst->word != 12 && ((opcode == OP_RTYPE && (i==31-5 || i==31-10 || i==31-15 || i==31-20 || i==31-25)) ||
			//check for only J type
			(opcode == OP_JAL && i==31-25) ||
			//all other are I type
			(opcode != OP_RTYPE && opcode != OP_JAL && (i==31-15 || i==31-20 || i==31-25)))) {
			printf(" ");
		}
	}
	printf("\n");
}

void printInstHex(union mips_instruction* inst) {
	//check for syscall
	if(inst->word == 12) {
		printf("DEBUG syscall\n");
		return;
	}
	// R type
	if(inst->rtype.opcode == OP_RTYPE) {
		printf("DEBUG R type ");
		printf("opcode:0x%X ",inst->rtype.opcode);
		printf("rs:0x%X ",inst->rtype.rs);
		printf("rt:0x%X ",inst->rtype.rt);
		printf("rd:0x%X ",inst->rtype.rd);
		printf("shamt:0x%X ",inst->rtype.shamt);
		printf("func:0x%X",inst->rtype.func);
	// only J type to check for
	} else if(inst->jtype.opcode == OP_JAL) {
		printf("DEBUG J type ");
		printf("opcode:0x%X ", inst->jtype.opcode);
		printf("addr:0x%X ", inst->jtype.addr);
	// not R type or the only J type, must be I type, could also do or on I types but lazy
	} else if(inst->rtype.opcode != OP_RTYPE && inst->itype.opcode != OP_JAL) {
		printf("DEBUG I type ");
		printf("opcode:0x%X ", inst->itype.opcode);
		printf("rs:0x%X ", inst->itype.rs);
		printf("rt:0x%X ", inst->itype.rt);
		printf("imm:0x%X", inst->itype.imm);
	}
	printf("\n");
}

/**
	@brief Simulates a single instruction
	
	Return 0 to exit the program (for syscall/invalid instruction) and 1 to keep going
 */
int SimulateInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx)
{
	//print the instruction so we know what hte heck we're supposed to be doing
	// printInstBits(inst);
	printInstHex(inst);
	int result;
	if(inst->word == 12) {
		result = SimulateSyscall(ctx->regs[2], memory, ctx);
	} else if(inst->rtype.opcode == OP_RTYPE) {
		result = SimulateRtypeInstruction(inst, memory, ctx);
	// not R type or the only J type, must be I type, could also do or on I types but lazy
	} else if(inst->rtype.opcode != OP_RTYPE && inst->itype.opcode != OP_JAL) {
		result = SimulateItypeInstruction(inst, memory, ctx);
	// check for j type
	} else if(inst->jtype.opcode == OP_JAL) {
		result = SimulateJtypeInstruction(inst, memory, ctx);
		//do something fancy with jumps, don't worry about it now
		ctx->pc += 4;
	}
	//for now I'm feeling lazy and this always gets bumped by 4, to jump just remember to decrement by 4
	ctx->pc += 4;
	return result;
}

int SimulateRtypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx)
{
	//instructions to impliment 
	// R ALU: ADD, ADDU, AND, OR, SUB, SUBU, XOR, SLT, SLTI, SLTIU, SLTU, SLL, SLLV, SRA, SRL, SRLV, DIV, DIVU, MULT, MULTU
	// R move: MFHI, MFLO
	switch(inst->rtype.opcode) {
		default:
			printf("GOT A BAD R TYPE INSTRUCIONT");
			return 0; //return this to exit program
	}

	return 1;
}

int SimulateItypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx)
{
	// instructions to impliment
	// I ALU: ADDI, ADDIU, ANDI, LUI, ORI, XORI
	// I branch: BEQ, BGEZ, BGTZ, BLEZ, BLTZ, BNE, BGEZAL, BLTZAL
	// I load/store: LB, LW, SB, SW
	switch(inst->itype.opcode) {
		case OP_ADDIU:	//R[rt] = R[rs] + SignExtImm
			ctx->regs[inst->itype.rt] = ctx->regs[inst->itype.rs] + inst->itype.imm;
			break;
		case OP_LUI:

			break;
		case OP_LW:

			break;
		case OP_SW:

			break;
		default:
			printf("GOT A BAD I TYPE INSTRUCITON");
			return 0; //return this to exit program
	}
	return 1;
}

int SimulateJtypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx)
{
	//do some weird shit, not sure how pc reg is gonna work yet
	// instructions to impliment
	// J jump: J, JAL, JR
	return 1;
}

int SimulateSyscall(uint32_t callnum, struct virtual_mem_region* memory, struct context* ctx)
{
	printf("Simulating syscall #%d\n", callnum);
	switch(callnum) {
		case 10: //exit program
			return 0;
			break;
		default:
			printf("GOT A BAD SYSCALL");
			return 0;
	}
	return 1;
}
