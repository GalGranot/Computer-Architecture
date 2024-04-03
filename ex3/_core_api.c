/* 046267 Computer Architecture - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>

typedef struct _thread
{
	bool valid;
	int id;
	int pc;
	int waitingforClk;
} Thread;

bool isShortCmd(cmd_opcode op)
{
	return (op == CMD_NOP ||
			op == CMD_ADD ||
			op == CMD_SUB ||
			op == CMD_ADDI ||
			op == CMD_SUBI);
}

void CORE_BlockedMT() 
{
	int clk = 0;
	int threadsNum = SIM_GetThreadsNum();
	Thread* threads = (Thread*)malloc(sizeof(Thread) * threadsNum);
	for(int i = 0; i < threadsNum; i++)
		threads[i] = (Thread){ .valid = true, .id = i, .pc = 0, .waitingforClk = 0};
	
	tcontext context;
	for(int i = 0; i < REGS_COUNT; i++)
		context.reg[i] = 0;
	Instruction* ins;
	Thread thread = threads[0];
	while(1)
	{
		SIM_MemInstRead(thread.pc, ins, thread.id);
		cmd_opcode op = ins->opcode;
		thread.pc++;
		if(isShortCmd(op))
		{
			if(op == CMD_NOP)
				continue;
			//cmd is either add/addi/sub/subi
			int result = context.reg[ins->src1_index];
			int secondOperand = ins->isSrc2Imm ? ins->src2_index_imm : context.reg[ins->src2_index_imm];
			secondOperand *= (op == CMD_SUB || op == CMD_SUBI) ? -1 : 1;
			result += secondOperand;
			continue;
		}
		else if(op != CMD_HALT)
		{
			if(op == CMD_STORE)
				SIM_MemDataWrite(ins->dst_index + ins->src2_index_imm, context.reg[ins->src1_index]);
			else if(op == CMD_LOAD)
				SIM_MemDataRead(ins->src1_index + ins->src2_index_imm, &context.reg[ins->dst_index]);
			thread.waitingforClk = clk + (op == CMD_STORE ? SIM_GetStoreLat() : SIM_GetLoadLat());
			thread.valid = false;
			for(int i = 0; i < threadsNum; i++) //try switching to another thread
			{
				Thread maybeThread = threads[(thread.id + i) % threadsNum];
				if(maybeThread.valid)
				{
					thread = maybeThread;
					break;
				}
			}
		}
		else //halt
		{
			thread.valid = false;
			for(int i = 0; i < threadsNum; i++)
			{
				Thread maybeThread = threads[(thread.id + i) % threadsNum];
				if(maybeThread.valid || clk >= maybeThread.waitingforClk);
				{
					thread = maybeThread;
					break;
				}
				//update statistics
				free(threads);
				return;
			}
		}
		clk++;
		//complete ins
		//decide if context switch
		//update statistics
	}
}

void CORE_FinegrainedMT() {
}

double CORE_BlockedMT_CPI(){
	return 0;
}

double CORE_FinegrainedMT_CPI(){
	return 0;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}
