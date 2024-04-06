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

void finishCycle(Thread* threads)
{
	for(int i = 0; i < SIM_GetThreadsNum(); i++) //decrement all counters
	{
		if(threads[i].stalledFor > 0)
		{
			if(debug) printf("thread %d stalling for %d\n", threads[i].id, threads[i].stalledFor);
			threads[i].stalledFor--;
		}
	}
	cycles++;
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
			finishCycle(threads);
			continue;
		}
		else if(thread->halt) //try to switch if possible, else finish
		{
			//if(debug) printf("thread %d entered cmd when halted\n", thread->id);
			bool foundNotHalted = false;
			for(int i = 1; i < threadsNum; i++)
			{
				Thread* maybeThread = &threads[(thread->id + i) % threadsNum];
				if(!maybeThread->halt)
					foundNotHalted = true;
				if(maybeThread->halt || maybeThread->stalledFor > 0)
					continue;
				if(debug) printf("thread %d changed to thread %d\n", thread->id, maybeThread->id);
				thread = maybeThread;
				break;
			}
			if(!thread->halt || foundNotHalted)
			{
				finishCycle(threads);
				continue;
			}
			free(ins);
			free(threads);
			return;
		}
		SIM_MemInstRead(thread->pc++, ins, thread->id); //thread is open to receive new ins
		finishCycle(threads);
		//if(debug) printf("read ins in pc %d in thread id %d\n", thread->pc - 1, thread->id);
		insCount++;
		cmd_opcode op = ins->opcode;
		int* tReg = contexts[thread->id].reg;
		int src1 = tReg[ins->src1_index];
		int src2 = ins->isSrc2Imm ? ins->src2_index_imm : tReg[ins->src2_index_imm];
		if(op == CMD_NOP)
			continue;
		else if(op == CMD_ADD || op == CMD_ADDI || op == CMD_SUB || op == CMD_SUBI)
		{
			if(debug) 
			{
				printf("cycle %d thread %d: arithmetic cmd\n", cycles, thread->id);
				printf("src1 = %d, src2 = %d\n", src1, src2);
			}
			int* dst = &tReg[ins->dst_index];
			if(op == CMD_ADD || op == CMD_ADDI)
				*dst = src1 + src2;
			else
				*dst = src1 - src2;
			continue;
		}
		else if(op != CMD_HALT) //store/load 
		{
			if(debug) printf("cycle %d thread %d: load/store cmd\n", cycles, thread->id);
			if(op == CMD_STORE)
			{
				SIM_MemDataWrite(tReg[ins->dst_index] + src2, src1);
			}
			else if(op == CMD_LOAD)
			{
				SIM_MemDataRead(src1 + src2, &tReg[ins->dst_index]);
			}
			thread->stalledFor = op == CMD_STORE ? SIM_GetStoreLat() : SIM_GetLoadLat();
			continue;
		}
		else if(op == CMD_HALT)
		{
			if(debug) printf("cycle %d thread %d: halt cmd\n", cycles, thread->id);
			thread->halt = true;
		}
	}
}

void CORE_FinegrainedMT() 
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
		if (thread->stalledFor == 0 && !thread->halt)
		{
			// do instruction
			SIM_MemInstRead(thread->pc++, ins, thread->id); //thread is open to receive new ins
			//if(debug) printf("read ins in pc %d in thread id %d\n", thread->pc - 1, thread->id);
			insCount++;
			cmd_opcode op = ins->opcode;
			int* tReg = contexts[thread->id].reg;
			int src1 = tReg[ins->src1_index];
			int src2 = ins->isSrc2Imm ? ins->src2_index_imm : tReg[ins->src2_index_imm];
			if(op == CMD_NOP)
				;// continue;
			else if(op == CMD_ADD || op == CMD_ADDI || op == CMD_SUB || op == CMD_SUBI)
			{
				if(debug) 
				{
					printf("cycle %d thread %d: arithmetic cmd\n", cycles, thread->id);
					printf("src1 = %d, src2 = %d\n", src1, src2);
				}
				int* dst = &tReg[ins->dst_index];
				if(op == CMD_ADD || op == CMD_ADDI)
					*dst = src1 + src2;
				else
					*dst = src1 - src2;
				// continue;
			}
			else if(op != CMD_HALT) //store/load 
			{
				if(debug) printf("cycle %d thread %d: load/store cmd\n", cycles, thread->id);
				if(op == CMD_STORE)
				{
					SIM_MemDataWrite(tReg[ins->dst_index] + src2, src1);
				}
				else if(op == CMD_LOAD)
				{
					SIM_MemDataRead(src1 + src2, &tReg[ins->dst_index]);
				}
				thread->stalledFor = 1+ (op == CMD_STORE ? SIM_GetStoreLat() : SIM_GetLoadLat());
				// continue;
			}
			else if(op == CMD_HALT)
			{
				if(debug) printf("cycle %d thread %d: halt cmd\n", cycles, thread->id);
				thread->halt = true;
			}
		}

		finishCycle(threads);

		bool foundNotStalled = false;
		bool switched = false;

		// try to switch 
		for(int i = 1; i < threadsNum; i++)
		{
			Thread* maybeThread = &threads[(thread->id + i) % threadsNum];

			if(!maybeThread->halt && maybeThread->stalledFor == 0)
			{ // found switch
				if(debug) printf("thread %d changed to thread %d\n", thread->id, maybeThread->id);
				thread = maybeThread;
				switched = true;
				break;
			}

			if (!switched && maybeThread->stalledFor > 0 && !maybeThread->halt)
			{
				foundNotStalled = true;
			}
		}
		
		if(!foundNotStalled && thread->halt) // if finished thread
		{
			free(ins);
			free(threads);
			return;
		}
	}
}

double CORE_BlockedMT_CPI()
{
	free(contexts);
	double cpi = (double)cycles / (double)insCount;
	cycles = 0;
	insCount = 0;
	return cpi;
}

double CORE_FinegrainedMT_CPI()
{
	free(contexts);
	double cpi = (double)cycles / (double)insCount;
	cycles = 0;
	insCount = 0;
	return cpi;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid)
{
	context[threadid] = contexts[threadid];
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) 
{
	context[threadid] = contexts[threadid];
}
