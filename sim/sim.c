/**
	@file
	@author Andrew D. Zonenberg
	@brief The core of the simulator
 */
#include "sim.h"
// I like my bools
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
	FILE* debug_log_file = fopen("sim_debug_log.txt", "w");
	FILE* log_file = fopen("simlog.txt", "w");
	struct logging_counters counters;
	counters.rtypes = 0;
	counters.itypes = 0;
	counters.jtypes = 0;
	counters.syscalls = 0;
	counters.elapsed_time = 0;
	clock_in(&counters);

	union mips_instruction inst;
	while(1)
	{
		//print pc counter in the debug log
		fprintf(debug_log_file, "(0x%X) ", ctx->pc);

		inst.word = FetchWordFromVirtualMemory(ctx->pc, memory);
		if(!SimulateInstruction(&inst, memory, ctx, &counters, &debug_log_file))
			break;
	}

	// clock out
	clock_out(&counters);

	// write to log flie
	print_log(&counters, &log_file);

	// close files
	fclose(debug_log_file);
	fclose(log_file);
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
	} else if((4 <= inst->itype.opcode && inst->itype.opcode <= 43) || inst->itype.opcode == 0x01) {
		// i type
		return 3;
	} else if(2 <= inst->jtype.opcode && inst->jtype.opcode <= 3) {
		// j type
		return 4;
	} else {
		return 0;
	}
}

void printInstBits(union mips_instruction* inst, struct logging_counters* counters, FILE** debug_log_file) {
	uint32_t inst_word = inst->word;
	fprintf(*debug_log_file, "DEBUG inst: ");
	int i;
	uint32_t reversed = 0;
	//because I'm really freaking lazy
	for(i=0; i<sizeof(inst_word)*8; ++i) {
		reversed += (inst_word%2)<<(sizeof(inst_word)*8-i-1);
		inst_word = inst_word>>1;
	}
	int type = determineInstType(inst);
	for(i=0; i<sizeof(reversed)*8; ++i) {
		fprintf(*debug_log_file, "%d", reversed%2);
		reversed = reversed>>1;
		//add spacing so it's easier to read, first check for R type
		if(type != 1 && ((type == 2 && (i==31-6 || i==31-11 || i==31-16 || i==31-21 || i==31-26)) ||
			//check for only J type
			(type == 4 && i==31-26) ||
			//all other are I type
			(type == 3 && (i==31-16 || i==31-21 || i==31-26)) ) ) {
			fprintf(*debug_log_file, " ");
		}
	}
	fprintf(*debug_log_file, "\n");
}

void printInstHex(union mips_instruction* inst, struct logging_counters* counters, FILE** debug_log_file) {
	int type = determineInstType(inst);
	//check for syscall
	if(type == 1) {
		fprintf(*debug_log_file, "DEBUG syscall\n");
		return;
	// R type
	} else if(type == 2) {
		fprintf(*debug_log_file, "DEBUG R type ");
		fprintf(*debug_log_file, "opcode:0x%02X ",inst->rtype.opcode);
		fprintf(*debug_log_file, "rs:0x%02X ",inst->rtype.rs);
		fprintf(*debug_log_file, "rt:0x%02X ",inst->rtype.rt);
		fprintf(*debug_log_file, "rd:0x%02X ",inst->rtype.rd);
		fprintf(*debug_log_file, "shamt:0x%02X ",inst->rtype.shamt);
		fprintf(*debug_log_file, "func:0x%02X",inst->rtype.func);
	// I type
	} else if(type == 3) {
		fprintf(*debug_log_file, "DEBUG I type ");
		fprintf(*debug_log_file, "opcode:0x%02X ", inst->itype.opcode);
		fprintf(*debug_log_file, "rs:0x%02X ", inst->itype.rs);
		fprintf(*debug_log_file, "rt:0x%02X ", inst->itype.rt);
		fprintf(*debug_log_file, "imm:0x%04X", inst->itype.imm);
	// J type
	} else if(type == 4) {
		fprintf(*debug_log_file, "DEBUG J type ");
		fprintf(*debug_log_file, "opcode:0x%02X ", inst->jtype.opcode);
		fprintf(*debug_log_file, "addr:0x%07X ", inst->jtype.addr<<2);
	} else {
		fprintf(*debug_log_file, "DEBUG unknown type ");
		//this should always work
		printInstBits(inst, counters, debug_log_file);
	}
	fprintf(*debug_log_file, "\n");
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
int SimulateInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx, 
	struct logging_counters* counters, FILE** debug_log_file)
{
	//print the instruction so we know what hte heck we're supposed to be doing
	// printInstBits(inst, counters, debug_log_file);
	printInstHex(inst, counters, debug_log_file);

	//do some switching
	int result;
	int type = determineInstType(inst);
	if(type == 1) {
		counters->syscalls++;
		result = SimulateSyscall(memory, ctx, counters, debug_log_file);
	} else if(type == 2) {
		counters->rtypes++;
		result = SimulateRtypeInstruction(inst, memory, ctx);
	// not R type or the only J type, must be I type, could also do or on I types but lazy
	} else if(type == 3) {
		counters->itypes++;
		result = SimulateItypeInstruction(inst, memory, ctx);
	// check for j type
	} else if(type == 4) {
		counters->jtypes++;
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
	// R ALU: ADD, ADDU, AND, OR, SUB, SUBU, XOR, SLT, SLTU, SLL, SLLV, SRA, SRL, SRLV, DIV, DIVU, MULT, MULTU
	// R move: MFHI, MFLO
	// R jump: JR
	switch(inst->rtype.func) {
		// R ALU
		case 0x20: // Add R[rd] = R[rs] + R[rt]
			;
			uint64_t add_result = ctx->regs[inst->rtype.rs] + ctx->regs[inst->rtype.rt];
			ctx->regs[inst->rtype.rd] =  add_result;
			if((add_result & 0xFFFFFFFF00000000)) 		//Checks for overflow
				printf("WARNING: add overflow\n");
			break;
		case 0x22: // sub R[rd] = R[rs] - R[rt]
			;
			uint64_t sub_result = ctx->regs[inst->rtype.rs] - ctx->regs[inst->rtype.rt];
			ctx->regs[inst->rtype.rd] =  sub_result;
			if((sub_result & 0xFFFFFFFF00000000)) 		//Checks for overflow
				printf("WARNING: sub overflow\n");
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
		case 0x18:; //Mult {Hi,Lo} = R[rs] * R[rt]
			///this seems liike cheating, but lets try it for now
			int64_t total = ((int64_t) ctx->regs[inst->rtype.rt]) * ((int64_t) ctx->regs[inst->rtype.rs]);
			ctx->lo = (uint32_t) total & 0x00000000FFFFFFFF;
			ctx->hi = (uint32_t) ((total & 0xFFFFFFFF00000000)>>32);
			break;
		case 0x19:; //Multu {Hi,Lo} = R[rs] * R[rt]
			uint64_t utotal = ((uint64_t) ctx->regs[inst->rtype.rt]) * ((uint64_t) ctx->regs[inst->rtype.rs]);
			ctx->lo = (uint32_t) utotal & 0x00000000FFFFFFFF; // bottom 32 bits
			ctx->hi = (uint32_t) ((utotal & 0xFFFFFFFF00000000)>>32); // top 32 bits
			break; 
		case 0x1a: // div Lo=R[rs]/R[rt]; Hi=R[rs]%R[rt]
			if(ctx->regs[inst->rtype.rt] == 0) {
				printf("WARNING: tried to div by 0, not gonna do that\n");
				break;
			}
			ctx->lo = ((int32_t) ctx->regs[inst->rtype.rs]) / ((int32_t) ctx->regs[inst->rtype.rt]);
			ctx->hi = ((int32_t) ctx->regs[inst->rtype.rs]) % ((int32_t) ctx->regs[inst->rtype.rt]);
			break;
		case 0x1b: // divu Lo=R[rs]/R[rt]; Hi=R[rs]%R[rt]
			if(ctx->regs[inst->rtype.rt] == 0) {
				printf("WARNING: tried to div by 0, not gonna do that\n");
				break;
			}
			ctx->lo = ctx->regs[inst->rtype.rs] / ctx->regs[inst->rtype.rt];
			ctx->hi = ctx->regs[inst->rtype.rs] % ctx->regs[inst->rtype.rt];
			break;
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
		case 0x2B: //SLTU R[rd]=R[rs] SLTU R[rt]. registers are unsigned by default
			if (ctx->regs[inst->rtype.rs] < ctx->regs[inst->rtype.rt]) {
				ctx->regs[inst->rtype.rd] = 1;
			} else {
				ctx->regs[inst->rtype.rd] = 0;
			}
			break;
			//shifts debugging these
		case 0x00: //SLL R[rd]=R[rs] << shamt. Constant input (shamt)
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rt] << inst->rtype.shamt;
			break;
		case 0x02: //SRL R[rd]=R[rs] >> shamt. Constant (shamt)
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rt] >> inst->rtype.shamt;
			break;
		case 0x03: //SRA R[rd]=R[rs] >> R[rt] (Sign-Extended).
			ctx->regs[inst->rtype.rd] = ((int32_t) ctx->regs[inst->rtype.rt]) >> inst->rtype.shamt;
			break;
		case 0x04: //SLLV. R[rd]=R[rt] << R[rs] (0-Extended) Variable input
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rt] << ctx->regs[inst->rtype.rs];
			break;
		case 0x06: //SRLV R[rd]=R[rt] >> R[rs] (0-Extended) Variable input
			ctx->regs[inst->rtype.rd] = ctx->regs[inst->rtype.rt] >> ctx->regs[inst->rtype.rs];
			break;
		case 0x07: //SRAV R[rd]=R[rt] >> R[rs] (Sign-Extended) Variable input. Like the other arithmetic shifts it is not implemented
			ctx->regs[inst->rtype.rd] = ((int32_t) ctx->regs[inst->rtype.rt]) >> ctx->regs[inst->rtype.rs];
			break;
		case 0x10: // mfhi: move from hi.
			ctx->regs[inst->rtype.rd] = ctx->hi;
			break;
		case 0x12: // mflo: move form lo
			ctx->regs[inst->rtype.rd] = ctx->lo;
			break;
		// R jump: JR
		case 0x08: // PC=R[rs]
			ctx->pc = ctx->regs[inst->rtype.rs];
			return 1; // early return to avoid ctx->pc += 4
			break; // for style
		case 0x0d:
			// printf("Got a breakpoint, just gonna ignore it for now\n");
			break;
		default:
			printf("Got a bad/unimplimented R type instruciton exiting\n");
			return 0; //return this to exit program
	}
	ctx->pc += 4;
	return 1;
}

int SimulateItypeInstruction(union mips_instruction* inst, struct virtual_mem_region* memory, struct context* ctx)
{
	// instructions to impliment
	// I ALU: ADDI, ADDIU, ANDI, LUI, ORI, XORIc, SLTI, SLTIU,
	// I branch: BEQ, BGEZ, BGTZ, BLEZ, BLTZ, BNE, BGEZAL, BLTZAL
	// I load/store: LB, LW, SB, SW
	int32_t imm_filled = signFill(inst->itype.imm, 16); // sign fill for convience
	switch(inst->itype.opcode) {
		case OP_ADDIU:	//R[rt] = R[rs] + SignExtImm
			//note this actually adds a signed number BUT doesn't throw anything when there's overflow
			ctx->regs[inst->itype.rt] = ctx->regs[inst->itype.rs] + imm_filled;
			break;
		case 0x08:	// addi Signed R[rt] = R[rs] + SignExtImm
			;
			uint64_t result = ctx->regs[inst->itype.rs] + imm_filled;
			ctx->regs[inst->itype.rt] = result;
			if((result & 0xFFFFFFFF00000000)) 		//Checks for overflow
				printf("WARNING: addi overflow\n");
			break;
		case 0x0C: //Andi R[rt]=R[rs] & Imm
			ctx->regs[inst->itype.rt] = ctx->regs[inst->itype.rs] & inst->itype.imm;
			break;
		case 0x0D: //Ori R[rt]=R[rs] | Imm
			ctx->regs[inst->itype.rt] = ctx->regs[inst->itype.rs] | inst->itype.imm;
			break;
		case 0x0E: //XORI R[rt]=R[rs] ^ Imm
			ctx->regs[inst->itype.rt] = ctx->regs[inst->itype.rs] ^ inst->itype.imm;
			break; 
		case 0x0A: //SLTI R[rt]=R[rs] SLTI Imm
			ctx->regs[inst->itype.rt] = (int32_t) ctx->regs[inst->itype.rs] < (int32_t) imm_filled;
			break;
		case 0x0B: //SLTIU R[rt]=R[rs] SLTIU Imm
			ctx->regs[inst->itype.rt] = ctx->regs[inst->itype.rs] < imm_filled;
			break;
		case OP_LUI: // R[rt] = {imm, 16’b0}
			//put the 16 bits from the imm into the 16 most sig bits of the target reg, rest are set 0
			ctx->regs[inst->itype.rt] = inst->itype.imm<<16;
			break;
		case OP_LW: // R[rt] = M[R[rs]+SignExtImm]
			ctx->regs[inst->itype.rt] = FetchWordFromVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, memory);
			break;
		case OP_SW: // M[R[rs]+SignExtImm] = R[rt]
			StoreWordToVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, ctx->regs[inst->itype.rt], memory);
			break;
		case 0x20: // lb: load byte R[rt] (7:0)=M[R[rs]+SignExtImm](7:0)
			;
			int32_t lb_temp = FetchWordFromVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, memory);
			lb_temp=lb_temp & !0x7F; //wipe out 7 smallest bytes
			lb_temp=lb_temp | (ctx->regs[inst->itype.rs]);
			ctx->regs[inst->itype.rt]=lb_temp;
			break;
		case 0x28: // sb: store byte M[R[rs]+SignExtImm](7:0) = R[rt](7:0)
			; // nop for switch
			int32_t sb_word = FetchWordFromVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, memory);
			sb_word = sb_word & !0x7F; // wipe out 7 smallest bytes
			sb_word = sb_word | (ctx->regs[inst->itype.rt] & 0x7F); // or with 7 smallest bytes of rt
			StoreWordToVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, sb_word, memory);
			break;
		// this is not required so not going to debug it
		/*case 0x29: //sh store halfword
			; int32_t sh_word = FetchWordFromVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, memory);
			sh_word = sh_word & !0xFFFF0000; // wipe out the 16 most significant bits
			sh_word = sh_word | (ctx->regs[inst->itype.rt] & 0xFFFF); // or with 16 least significant bits
			StoreWordToVirtualMemory(ctx->regs[inst->itype.rs] + inst->itype.imm, sh_word, memory);
			break;*/
		case 0x05: // bne if(R[rs]!=R[rt]) PC=PC+4+BranchAddr
			if(ctx->regs[inst->itype.rs] != ctx->regs[inst->itype.rt]) {
				ctx->pc = ctx->pc + 4 + (imm_filled<<2);
				return 1; // return early to prevent auto +4
			}
			break;
		case 0x04: // beq if(R[rs]==R[rt]) PC=PC+4+BranchAddr
			if(ctx->regs[inst->itype.rs] == ctx->regs[inst->itype.rt]) {
				ctx->pc = ctx->pc + 4 + (imm_filled<<2);
				return 1; // return early to prevent auto +4
			}
			break;
		case 0x01:  //BLTZAL, BGEZAL, BGEZX and BLTZ
			if(inst->itype.rt==0x1) {
				if((int32_t) ctx->regs[inst->itype.rs] >= 0) { //bgez
					ctx->pc = ctx->pc + 4 + (imm_filled<<2);
					return 1; // return early to prevent auto +4
				}
			} else if(inst->itype.rt==0x0) {
				if ((int32_t) ctx->regs[inst->itype.rs] < 0) { //bltz
					ctx->pc = ctx->pc + 4 + (imm_filled<<2);
					return 1; // return early to prevent auto +4
				}
			} else if(inst->itype.rt==0x11) { //BGEZAL
				if((int32_t) ctx->regs[inst->itype.rs]>=0){
					ctx->regs[ra]=ctx->pc+4;
					ctx->pc = ctx->pc + 4 + ((ctx->pc & 0xF0000000) | (inst->itype.imm<<2));
					return 1; //return early to prevent auto +4
				}
			} else if(inst->itype.rt==0x10) { //BLTZAL
				if((int32_t)ctx->regs[inst->itype.rs]<=0){
					ctx->regs[ra]=ctx->pc+4;
					ctx->pc = ctx->pc + 4 + ((ctx->pc & 0xF0000000) | (inst->itype.imm<<2));
					return 1; //return early to prevent auto +4
				}
			}
			else {
				printf("Got bad I type instruction with opcode 0x01\n");
				return 0;
			}
			break;
		case 0x06: //blez
			if(ctx->regs[inst->itype.rt]==0x0) {
				if((int32_t) ctx->regs[inst->itype.rs] <= 0) {
					ctx->pc = ctx->pc + 4 + (imm_filled<<2);
					return 1; // return early to prevent auto +4
				}
			} else {
				printf("Got bad I type instruction with opcode 0x06\n");
				return 0;
			}
			break;
		case 0x07: //bgtz
			if(ctx->regs[inst->itype.rt]==0x0) {
				if((int32_t) ctx->regs[inst->itype.rs] > 0) {
					ctx->pc = ctx->pc + 4 + (imm_filled<<2);
					return 1; // return early to prevent auto +4
				}
			} else {
				printf("Got bad I type instruction with opcode 0x07\n");
				return 0;
			}
			break;
		default:
			printf("Got a bad/unimplimented I type instruciton exiting\n");
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
		case 0x02: // j: PC=JumpAddr
			ctx->pc = (ctx->pc & 0xF0000000) | (inst->jtype.addr<<2);
			return 1; //early return to prevent pc+=4
			break; // style
		default:
			printf("Got a bad/unimplimented J type instruciton exiting\n");
			return 0;
	}
	ctx->pc += 4;
	return 1;
}

int SimulateSyscall(struct virtual_mem_region* memory, struct context* ctx, struct logging_counters* counters, FILE** debug_log_file)
{
	fprintf(*debug_log_file, "Simulating syscall #%d\n", ctx->regs[v0]);

	switch(ctx->regs[v0]) {
		case 1: //print int
			printf("%d",ctx->regs[a0]);
			break;
		case 4: //print null terminated string
			; //nop for switch
			uint32_t addr = ctx->regs[a0];
			int32_t word;
			bool done = false;
			while(!done && (word = FetchWordFromVirtualMemory(addr, memory))) {
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
		case 5: //read int
			// $v0 contains integer read
			// stop the clock
			clock_out(counters);
			int result_int;
			scanf("%d", &result_int);
			ctx->regs[v0] = result_int;
			// start the clock
			clock_in(counters);
			break;
		case 8: // read string
			// $a0 = address of input buffer (in real or fake life>>)
			// $a1 = maximum number of characters to read
			// implimentation is purley ceremonial becaues not going to alloc memmory because not worth it
			// TODO: some bs with scanf
			break;		
		case 10: //exit program
			return 0;
			break;
		case 11: // print char
			printf("%c", ctx->regs[a0]);
			break;
		case 12: // read char
			// $v0 contains character read
			// stop the clock
			clock_out(counters);
			char result_char;
			scanf("%c", &result_char);
			ctx->regs[v0] = result_char;
			// start the clock
			clock_in(counters);
			break;
		default:
			printf("GOT A BAD/UNIMPLIMENTED SYSCALL\n");
			return 0;
	}
	ctx->pc += 4;
	return 1;
}

void print_log(struct logging_counters* counters, FILE** log_file) {
	//elapsed time is in ns
	fprintf(*log_file, "Time elapsed (s): %f\n", ((float) counters->elapsed_time) / 1000000000);
	fprintf(*log_file, "\nInstructions simulated:\n");
	fprintf(*log_file, "    R type:   %d\n", counters->rtypes);
	fprintf(*log_file, "    I type:   %d\n", counters->itypes);
	fprintf(*log_file, "    J type:   %d\n", counters->jtypes);
	fprintf(*log_file, "    Syscalls: %d\n", counters->syscalls);
}

void clock_in(struct logging_counters* counters) {
	clock_gettime(CLOCK_REALTIME, &(counters->in_time));
}

void clock_out(struct logging_counters* counters) {
	clock_gettime(CLOCK_REALTIME, &(counters->out_time));
	counters->elapsed_time += counters->out_time.tv_nsec - counters->in_time.tv_nsec;
}
