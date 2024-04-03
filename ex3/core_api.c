/* 046267 Computer Architecture - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>

typedef struct _thread
{
	int id;
	int pc;
	int stalledFor;
	bool halt;
} Thread;

typedef struct _data
{
	double cycles;
	double ins;
} Data;
Data* data;
tcontext* contexts;


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
	data = (Data*)malloc(sizeof(Data));
	*data = (Data){.cycles = 0, .ins = 0};
	Instruction* ins = (Instruction*)malloc(sizeof(Instruction));
	Thread thread = threads[0];
	int k = 0;
	while(1)
	{
		printf("Cycle %d, thread %d\n", k++, thread.id);
		data->cycles++;
		for(int i = 0; i < threadsNum; i++) //decrement all counters
		{
			if(threads[i].stalledFor > 0)
				threads[i].stalledFor--;
		}
		if(thread.stalledFor > 0) //try to switch
		{
			for(int i = 1; i < threadsNum; i++)
			{
				Thread maybeThread = threads[(thread.id + i) % threadsNum];
				if(maybeThread.halt || maybeThread.stalledFor > 0)
					continue;
				thread = maybeThread;
				break;
			}
			continue;
		}
		else if(thread.halt) //try to switch if possible, else finish
		{
			for(int i = 1; i < threadsNum; i++)
			{
				Thread maybeThread = threads[(thread.id + i) % threadsNum];
				if(maybeThread.halt || maybeThread.stalledFor > 0)
					continue;
				thread = maybeThread;
				break;
			}
			if(!thread.halt)
				continue;
			free(ins);
			return;
		}
		SIM_MemInstRead(thread.pc++, ins, thread.id); //thread is open to receive new ins
		data->ins++;
		cmd_opcode op = ins->opcode;
		if(op == CMD_NOP)
			continue;
		else if(op == CMD_ADD || op == CMD_ADDI || op == CMD_SUB || op == CMD_SUBI)
		{
			int* result = &contexts[thread.id].reg[ins->src1_index];
			int secondOperand = ins->isSrc2Imm ? ins->src2_index_imm : contexts[thread.id].reg[ins->src2_index_imm];
			secondOperand *= (op == CMD_SUB || op == CMD_SUBI) ? -1 : 1;
			*result += secondOperand;
			continue;
		}
		else if(op != CMD_HALT) //store/load 
		{
			if(op == CMD_STORE)
				SIM_MemDataWrite(ins->dst_index + ins->src2_index_imm, contexts[thread.id].reg[ins->src1_index]);
			else if(op == CMD_LOAD)
				SIM_MemDataRead(ins->src1_index + ins->src2_index_imm, &contexts[thread.id].reg[ins->dst_index]);
			thread.stalledFor = op == CMD_STORE ? SIM_GetStoreLat() : SIM_GetLoadLat();
			continue;
		}
		else if(op == CMD_HALT)
		{
			thread.halt = true;
		}
	}
}

void CORE_FinegrainedMT() {
}

double CORE_BlockedMT_CPI(){
	return 0;
}

double CORE_FinegrainedMT_CPI()
{
	double result = (double)(data->cycles / data->ins);
	free(data);
	free(contexts);
	return result;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid)
{
	for(int i = 0; i < REGS_COUNT; i++)
		context->reg[i] = contexts[threadid].reg[i];
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}
