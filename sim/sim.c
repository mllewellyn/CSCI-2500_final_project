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
	returns int based on what type of instruction it is
	0 error
	1 syscall
	2 R type
	3 I type
	4 J type
*/
int determineInstType(union mips_instruction* inst) {
	if(inst->word == 12) {
		//syscall
		return 1;
	} else if(inst->rtype.opcode == OP_RTYPE) {
		// r type
		return 2;
	} else if(4 <= inst->itype.opcode && inst->itype.opcode <= 43) {
		// i type
		return 3;
	} else if(2 <= inst->jtype.opcode && inst->jtype.opcode <= 3) {
		// j type
		return 4;
	} else {
		return 0;
	}
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
		//print pc counter for debugging
		printf("(%X) ", ctx->pc);
		inst.word = FetchWordFromVirtualMemory(ctx->pc, memory);
		if(!SimulateInstruction(&inst, memory, ctx))
			break;		
	}
}

void printInstBits(union mips_instruction* inst) {
	uint32_t inst_word = inst->word;
	printf("DEBUG inst: ");
	int i;
	uint32_t reversed = 0;
	//because I'm really freaking lazy
	for(i=0; i<sizeof(inst_word)*8; ++i) {
		reversed += (inst_word%2)<<(sizeof(inst_word)*8-i-1);
		inst_word = inst_word>>1;
	}
	int type = determineInstType(inst);
	for(i=0; i<sizeof(reversed)*8; ++i) {
		printf("%d", reversed%2);
		reversed = reversed>>1;
		//add spacing so it's easier to read, first check for R type
		if(type != 1 && ((type == 2 && (i==31-5 || i==31-10 || i==31-15 || i==31-20 || i==31-25)) ||
			//check for only J type
			(type == 4 && i==31-25) ||
			//all other are I type
			(type != 3 && (i==31-15 || i==31-20 || i==31-25)))) {
			printf(" ");
		}
	}
	printf("\n");
}

void printInstHex(union mips_instruction* inst) {
	int type = determineInstType(inst);
	//check for syscall
	if(type == 1) {
		printf("DEBUG syscall\n");
		return;
	// R type
	} else if(type == 2) {
		printf("DEBUG R type ");
		printf("opcode:0x%X ",inst->rtype.opcode);
		printf("rs:0x%X ",inst->rtype.rs);
		printf("rt:0x%X ",inst->rtype.rt);
		printf("rd:0x%X ",inst->rtype.rd);
		printf("shamt:0x%X ",inst->rtype.shamt);
		printf("func:0x%X",inst->rtype.func);
	// I type
	} else if(type == 3) {
		printf("DEBUG I type ");
		printf("opcode:0x%X ", inst->itype.opcode);
		printf("rs:0x%X ", inst->itype.rs);
		printf("rt:0x%X ", inst->itype.rt);
		printf("imm:0x%X", inst->itype.imm);
	// J type
	} else if(type == 4) {
		printf("DEBUG J type ");
		printf("opcode:0x%X ", inst->jtype.opcode);
		printf("addr:0x%X ", inst->jtype.addr);
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

	//do some switching
	int result;
	int type = determineInstType(inst);
	if(type == 1) {
		result = SimulateSyscall(ctx->regs[2], memory, ctx);
	} else if(type == 2) {
		result = SimulateRtypeInstruction(inst, memory, ctx);
	// not R type or the only J type, must be I type, could also do or on I types but lazy
	} else if(type == 3) {
		result = SimulateItypeInstruction(inst, memory, ctx);
	// check for j type
	} else if(type == 4) {
		result = SimulateJtypeInstruction(inst, memory, ctx);
	} else {
		printf("Couldn't find the type of instruction THIS IS AN ERROR, exiting");
		return 0;
	}
	return result;
}

int SimulateRtypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx)
{
	//instructions to impliment 
	// R ALU: ADD, ADDU, AND, OR, SUB, SUBU, XOR, SLT, SLTI, SLTIU, SLTU, SLL, SLLV, SRA, SRL, SRLV, DIV, DIVU, MULT, MULTU
	// R move: MFHI, MFLO
	// R jump: JR
	switch(inst->rtype.func) {
		case 0x20: // Add R[rd] = R[rs] + R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] + ctx->regs[inst->rtype.rt];
			break;
		case 0x22: // sub R[rd] = R[rs] - R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] - ctx->regs[inst->rtype.rt];
			break;
		case 0x24: // and R[rd] = R[rs] & R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] & ctx->regs[inst->rtype.rt];
			break;
		case 0x25: // or R R[rd] = R[rs] | R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] | ctx->regs[inst->rtype.rt];
			break;
		default:
			printf("GOT A BAD/UNIMPLIMENTED R TYPE INSTRUCIONT\n");
			return 0; //return this to exit program
	}
	ctx->pc += 4;
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

			// break;
		case OP_LW: // R[rt] = M[R[rs]+SignExtImm]
			// THIS IS NOT WORKING YET, WEIRD LABEL SHIT
			// printf("DEBUG SP addr:0x%X\n", ctx->regs[sp]);
			// printf("DEBUG MEM TARGET:0x%X\n", ctx->regs[inst->itype.rs] + inst->itype.imm + ctx->regs[sp]);
			// ctx->regs[inst->itype.rt] = FetchWordFromVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm + ctx->regs[sp], memory);
			// break;
		case OP_SW:

			// break; //removing break until implimented
		default:
			printf("GOT A BAD/UNIMPLIMENTED I TYPE INSTRUCITON\n");
			return 0; //return this to exit program
	}
	ctx->pc += 4;
	return 1;
}

int SimulateJtypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx)
{
	// remember ctx->pc is always bumped by 4 at the end of the simulate function, so some gettoh may be required bc I'm lazy

	// instructions to impliment
	// J jump: J, JAL
	switch(inst->jtype.opcode) {
		case OP_JAL: //R[ra]=PC+8;PC=JumpAddr
			ctx->regs[ra] = ctx->pc+8;
			ctx->pc = inst->jtype.addr;
			return 1; // early return to prevent pc+=4
			break; // just for style
		default:
			printf("GOT A BAD/UNIMPLIMENTED J TYPE INSTRUCITON\n");
			return 0;
	}
	ctx->pc += 4;
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
			printf("GOT A BAD/UNIMPLIMENTED SYSCALL\n");
			return 0;
	}
	ctx->pc += 4;
	return 1;
}
