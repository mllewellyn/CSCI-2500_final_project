/**
	@file
	@author Andrew D. Zonenberg
	@brief The core of the simulator
 */
#include "sim.h"
#include <stdbool.h>
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
	printf("SEGFAULT: attempted to read word from nonexistent virtual address 0x%08x\n", address);
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
			printf("SEGFAULT: address 0x%08x is not aligned\n", address);
			exit(1);	
		}
		
		memory->data[offset/4] = value;
		return;
	}
	
	//Didn't find anything! Give up
	printf("SEGFAULT: attempted to write word to nonexistent virtual address 0x%08x\n", address);
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
		//print pc counter for debugging
		printf("(0x%X) ", ctx->pc);
		inst.word = FetchWordFromVirtualMemory(ctx->pc, memory);
		if(!SimulateInstruction(&inst, memory, ctx))
			break;
	}
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
		if(type != 1 && ((type == 2 && (i==31-6 || i==31-11 || i==31-16 || i==31-21 || i==31-26)) ||
			//check for only J type
			(type == 4 && i==31-26) ||
			//all other are I type
			(type == 3 && (i==31-16 || i==31-21 || i==31-26)) ) ) {
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
		printf("opcode:0x%02X ",inst->rtype.opcode);
		printf("rs:0x%02X ",inst->rtype.rs);
		printf("rt:0x%02X ",inst->rtype.rt);
		printf("rd:0x%02X ",inst->rtype.rd);
		printf("shamt:0x%02X ",inst->rtype.shamt);
		printf("func:0x%02X",inst->rtype.func);
	// I type
	} else if(type == 3) {
		printf("DEBUG I type ");
		printf("opcode:0x%02X ", inst->itype.opcode);
		printf("rs:0x%02X ", inst->itype.rs);
		printf("rt:0x%02X ", inst->itype.rt);
		printf("imm:0x%04X", inst->itype.imm);
	// J type
	} else if(type == 4) {
		printf("DEBUG J type ");
		printf("opcode:0x%02X ", inst->jtype.opcode);
		printf("addr:0x%07X ", inst->jtype.addr<<2);
	}
	printf("\n");
}

/*
	Sign fill a number and return it
*/
int32_t signFill(int32_t val, int orig_bits) {
	val = val << 32 - orig_bits;
	val = val >> 32 - orig_bits;
	return val;
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
		result = SimulateSyscall(memory, ctx);
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
		// R ALU
		case 0x20: // Add R[rd] = R[rs] + R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] + ctx->regs[inst->rtype.rt];
			if(ctx->regs[inst->rtype.rd]>2147483647) {		//Checks for overflow
				return 0;
			}
			break;
		case 0x22: // sub R[rd] = R[rs] - R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] - ctx->regs[inst->rtype.rt];
			if(ctx->regs[inst->rtype.rd]<-2147483649) {		//Checks for overflow
				return 0;
			}
			break;
		case 0x24: // and R[rd] = R[rs] & R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] & ctx->regs[inst->rtype.rt];
			break;
		case 0x25: // or R R[rd] = R[rs] | R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] | ctx->regs[inst->rtype.rt];
			break;
		case 0x21: // Addu R[rd]=R[rs]+R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] + ctx->regs[inst->rtype.rt];
			break;
		case 0x23: //Subu  R[rd] = R[rs] - R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] - ctx->regs[inst->rtype.rt];
			break;
		/*case 0x18: //Mult R[rd] = R[rs] * R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] * ctx->regs[inst->rtype.rt];
			if(ctx->regs[inst->rtype.rd]<-2147483649||ctx->regs[inst->rtype.rd]>2147483647){		//Checks for overflow
				return 0;
			}
			break; 
		case 0x19: //Multu R[rd] = R[rs] * R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] * ctx->regs[inst->rtype.rt];
			break; */
		case 0x26: //XOR R[rd]=R[rs] ^ R[rt]
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rs] ^ ctx->regs[inst->rtype.rt];
			break;
		case 0x2A: //SLT R[rd]=R[rs] SLT R[rt]
			if (ctx->regs[inst->rtype.rs] < ctx->regs[inst->rtype.rt]) {
				ctx->regs[inst->rtype.rd] = 1;
			} else {
				ctx->regs[inst->rtype.rd] = 0;
			}
			break;
		case 0x2B: //SLTU R[rd]=R[rs] SLTU R[rt]. Unsigned component not implmenented, not sure how to cast labels as unsigned.
			if (ctx->regs[inst->rtype.rs] < ctx->regs[inst->rtype.rt]) {
				ctx->regs[inst->rtype.rd] = 1;
			} else {
				ctx->regs[inst->rtype.rd] = 0;
			}
			break;
		case 0x00: //SLL R[rd]=R[rs] << R[rt] (O-Extended). Constant input (shamt)
			ctx->regs[inst->rtype.rd] =ctx->regs[inst->rtype.rs]<<ctx->regs[inst->rtype.shamt];
			break;
		case 0x02: //SRL R[rd]=R[rs] >> R[rt] (0-Extended). Constant (shamt)
			ctx->regs[inst->rtype.rd] =ctx->regs[inst->rtype.rs]>>ctx->regs[inst->rtype.shamt];
			break;
		case 0x03: //SRA R[rd]=R[rs] >> R[rt] (Sign-Extended). Sign extension not implemented. Need to figure out casting labels as unsigned.
			ctx->regs[inst->rtype.rd] =ctx->regs[inst->rtype.rs]>>ctx->regs[inst->rtype.shamt];
			break;
		case 0x04: //SLLV. R[rd]=R[rs] << R[rt] (O-Extended) Variable input
			ctx->regs[inst->rtype.rd] =ctx->regs[inst->rtype.rs]<<ctx->regs[inst->rtype.rt];
			break;
		case 0x06: //SRLV R[rd]=R[rs] >> R[rt] (0-Extended) Variable input
			ctx->regs[inst->rtype.rd] =ctx->regs[inst->rtype.rs]>>ctx->regs[inst->rtype.rt];
			break;
		case 0x07: //SRAV R[rd]=R[rs] >> R[rt] (0-Extended) Variable input. Like the other arithmetic shifts it is not implemented
			ctx->regs[inst->rtype.rd] =ctx->regs[inst->rtype.rs]>>ctx->regs[inst->rtype.rt];
			break;
		
		default:
			printf("GOT A BAD/UNIMPLIMENTED R TYPE INSTRUCITON\n");
			return 0; //return this to exit program

		// R jump: JR
		case 0x08: // PC=R[rs]
			ctx->pc = ctx->regs[inst->rtype.rs];
			return 1; // early return to avoid ctx->pc += 4
			break; // for style
	}
	ctx->pc += 4;
	return 1;
}

int SimulateItypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx)
{
	// instructions to impliment
	// I ALU: ADDI, ADDIU, ANDI, LUI, ORI, XORIc
	// I branch: BEQ, BGEZ, BGTZ, BLEZ, BLTZ, BNE, BGEZAL, BLTZAL
	// I load/store: LB, LW, SB, SW
	switch(inst->itype.opcode) {
		case OP_ADDIU:	//R[rt] = R[rs] + SignExtImm
			//note this actually adds a signed number BUT doesn't throw anything when there's overflow
			; // nop to make the switch happy
			int32_t imm = signFill(inst->itype.imm, 16);
			// printf("DEBUG ORIG: 0x%x\n" ,ctx->regs[inst->itype.rt]);
			// printf("DEBUG FINAL 0x%x\n", ctx->regs[inst->itype.rs] + imm);
			ctx->regs[inst->itype.rt] = ctx->regs[inst->itype.rs] + imm;
			break;
		case OP_LUI: // R[rt] = {imm, 16’b0}
			//put the 16 bits from the imm into the 16 most sig bits of the target reg, rest are set 0
			ctx->regs[inst->itype.rt] = inst->itype.imm<<16;
			break;
		case OP_LW: // R[rt] = M[R[rs]+SignExtImm]
			// (note to self) WORKING DON't FUCK WITH
			// printf("DEBUG MEM TARGET:0x%X\n", ctx->regs[inst->itype.rs] + inst->itype.imm);
			ctx->regs[inst->itype.rt] = FetchWordFromVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, memory);
			break;
		case OP_SW: // M[R[rs]+SignExtImm] = R[rt]
			// (note to self) WORKING DON't FUCK WITH
			// printf("DEBUG REG_S VAL:0x%x\n", ctx->regs[inst->itype.rs]);
			// printf("DEBUG MEM TARGET:0x%x\n", ctx->regs[inst->itype.rs] + inst->itype.imm);
			StoreWordToVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, ctx->regs[inst->itype.rt], memory);
			break;
		case 0x28: // sb: store byte M[R[rs]+SignExtImm](7:0) = R[rt](7:0)
			; // nop for switch
			int32_t word = FetchWordFromVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, memory);
			word = word & !0x7F; // wipe out 7 smallest bytes
			word = word | (ctx->regs[inst->itype.rt] & 0x7F); // or with 7 smallest bytes of rt
			StoreWordToVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, word, memory);
			break;
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
			ctx->regs[ra] = ctx->pc+4;
			//get first 4 bits from PC and pop them on the front of the 26 (shifted to 28) bits from addr
			ctx->pc = (ctx->pc & 0xF0000000) | (inst->jtype.addr<<2);
			return 1; // early return to prevent pc+=4
			break; // just for style
		default:
			printf("GOT A BAD/UNIMPLIMENTED J TYPE INSTRUCITON\n");
			return 0;
	}
	ctx->pc += 4;
	return 1;
}

int SimulateSyscall(struct virtual_mem_region* memory, struct context* ctx)
{//uint32
	printf("Simulating syscall #%d\n", ctx->regs[v0]);
	switch(ctx->regs[v0]) {
		case 1: //print int
			printf("%d", FetchWordFromVirtualMemory(ctx->regs[a0], memory));
			printf("\n"); //this is just for debugging
			break;
		case 4: //print null terminated string
			; //nop for switch
			uint32_t addr = ctx->regs[a0];
			int32_t word;
			bool done = false;
			while((word = FetchWordFromVirtualMemory(addr, memory)) && !done ) {
				// read word then read chars from word
				for(int i=0; i<4; ++i) {
					//get the char (1 byte) from the word (4 bytes)
					char c = (word & (0xFF<<(i*8)))>>(i*8);
					if(c == 0) { //null terminate
						done = true;
						break;
					}
					printf("%c", c);
				}
				addr += 4;
			}
			break;
		case 10: //exit program
			return 0;
			break;
		case 12: // print char
			printf("%c", FetchWordFromVirtualMemory(ctx->regs[a0], memory));
			printf("\n"); //this is just for debugging
			break;
		default:
			printf("GOT A BAD/UNIMPLIMENTED SYSCALL\n");
			return 0;
	}
	ctx->pc += 4;
	return 1;
}
