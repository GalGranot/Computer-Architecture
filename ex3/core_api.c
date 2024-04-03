/* 046267 Computer Architecture - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>

#define debug 1

typedef struct _thread
{
	int id;
	int pc;
	int stalledFor;
	bool halt;
} Thread;

tcontext* contexts;
int insCount;
int cycles;

void decCtrs(Thread* threads)
{
	for(int i = 0; i < SIM_GetThreadsNum(); i++) //decrement all counters
	{
		if(threads[i].stalledFor > 0)
		{
			if(debug) printf("thread %d stalling for %d\n", threads[i].id, threads[i].stalledFor);
			threads[i].stalledFor--;
		}
	}
}

void CORE_BlockedMT() 
{
	int threadsNum = SIM_GetThreadsNum();
	Thread* threads = (Thread*)malloc(sizeof(Thread) * threadsNum);
	contexts = (tcontext*)malloc(sizeof(tcontext) * threadsNum);
	for(int i = 0; i < threadsNum; i++)
	{
		threads[i] = (Thread){.id = i, .pc = 0, .stalledFor = 0, .halt = false};
		for(int j = 0; j < REGS_COUNT; j++)
			contexts[i].reg[j] = 0;
	}
	Instruction* ins = (Instruction*)malloc(sizeof(Instruction));
	Thread* thread = &threads[0];
	insCount = 0;
	cycles = 0;
	int k = 0;
	while(1)
	{
		if(debug) printf("Cycle %d, thread %d\n", k++, thread->id);
		cycles++;
		for(int i = 0; i < threadsNum; i++) //decrement all counters
		{
			if(threads[i].stalledFor > 0)
			{
				if(debug) printf("thread %d stalling for %d\n", threads[i].id, threads[i].stalledFor);
				threads[i].stalledFor--;
			}
		}
		if(thread->stalledFor > 0) //try to switch
		{
			//if(debug) printf("thread %d is stalled for %d\n", thread->id, thread->stalledFor);
			for(int i = 1; i < threadsNum; i++)
			{
				Thread* maybeThread = &threads[(thread->id + i) % threadsNum];
				if(maybeThread->halt || maybeThread->stalledFor > 0)
					continue;
				if(debug) printf("thread %d changed to thread %d\n", thread->id, maybeThread->id);
				thread = maybeThread;
				break;
			}
			continue;
		}
		else if(thread->halt) //try to switch if possible, else finish
		{
			//if(debug) printf("thread %d entered cmd when halted\n", thread->id);
			for(int i = 1; i < threadsNum; i++)
			{
				Thread* maybeThread = &threads[(thread->id + i) % threadsNum];
				if(maybeThread->halt || maybeThread->stalledFor > 0)
					continue;
				if(debug) printf("thread %d changed to thread %d\n", thread->id, maybeThread->id);
				thread = maybeThread;
				break;
			}
			if(!thread->halt)
				continue;
			free(ins);
			return;
		}
		SIM_MemInstRead(thread->pc++, ins, thread->id); //thread is open to receive new ins
		//if(debug) printf("read ins in pc %d in thread id %d\n", thread->pc - 1, thread->id);
		insCount++;
		cmd_opcode op = ins->opcode;
		if(op == CMD_NOP)
			continue;
		else if(op == CMD_ADD || op == CMD_ADDI || op == CMD_SUB || op == CMD_SUBI)
		{
			if(debug) printf("cycle %d thread %d: arithmetic cmd\n", cycles-1, thread->id);
			int* result = &contexts[thread->id].reg[ins->dst_index];
			int secondOperand = ins->isSrc2Imm ? ins->src2_index_imm : contexts[thread->id].reg[ins->src2_index_imm];
			secondOperand *= (op == CMD_SUB || op == CMD_SUBI) ? -1 : 1;
			*result += secondOperand;
			continue;
		}
		else if(op != CMD_HALT) //store/load 
		{
			if(debug) printf("cycle %d thread %d: load/store cmd\n", cycles-1, thread->id);
			int* tReg = contexts[thread->id].reg;
			if(op == CMD_STORE)
			{
				uint32_t addr = tReg[ins->dst_index] + (ins->isSrc2Imm ? ins->src2_index_imm : tReg[ins->src2_index_imm]);
				int32_t val = tReg[ins->src1_index];
				SIM_MemDataWrite(addr, val);
			}
			else if(op == CMD_LOAD)
			{
				uint32_t addr = tReg[ins->src1_index] + (ins->isSrc2Imm ? ins->src2_index_imm : tReg[ins->src2_index_imm]);
				int32_t* val = &tReg[ins->dst_index];
				SIM_MemDataRead(addr, val);
			}
			thread->stalledFor = op == CMD_STORE ? SIM_GetStoreLat() : SIM_GetLoadLat();
			continue;
		}
		else if(op == CMD_HALT)
		{
			if(debug) printf("cycle %d thread %d: halt cmd\n", cycles-1, thread->id);
			thread->halt = true;
		}
	}
}

void CORE_FinegrainedMT() {
}

double CORE_BlockedMT_CPI()
{
	free(contexts);
	printf("\n\n%d %d\n\n", cycles, insCount);
	return (double)cycles / (double)insCount;
}

double CORE_FinegrainedMT_CPI()
{
	return 0;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid)
{
	context[threadid] = contexts[threadid];
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}
