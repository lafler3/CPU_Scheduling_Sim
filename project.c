//Project File Opsys
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
//Current Version

typedef struct{
	int* wait;
	int* turnaround;
	int* arrivalTimes;

	int waitSize;
	int turnSize;
	int arrivalTimeSize;

	int ID;
	
	int burstN;
	int tau;
	int prevActual;

	int procNum;
	int IONum;
	int* burst;
	int* IO;

	int arrivalTime;
	int ioEndTime;
	int burstEndTime;
	int suspended;

	double waitAVG;
	double turnAroundAVG;

}
queue_item;


int next_exps(double lambda, int threshED){
	int aT = threshED+1;
	while(aT > threshED){
		double r = drand48();
		aT=ceil(-log(r)/lambda);
	}
	return aT;
}


int *** next_exp(double lambda, int simN, int threshED){
	int*** data = calloc(simN, sizeof(int**));
	for(int i = 0; i<simN; i++){
		int aT = threshED+1;
		while(aT > threshED){
			double r = drand48();
			aT=floor(-log(r)/lambda);
		}
		int burstN = ceil(drand48()*100);
		int** tArray = calloc(3, sizeof(int*));
		int* aBD = calloc(3, sizeof(int));
		*(aBD + 0) = aT;
		*(aBD + 1) = burstN;
		*(aBD + 2) = 0;
		int* tCPU = calloc(burstN, sizeof(int));
		int* tIO = calloc((burstN-1), sizeof(int));
		int cTr = 0;
		int iTr = 0;
		for(int j = 0; j<(burstN+burstN-1); j++){
				if(j%2 == 0){
					*(tCPU + cTr) = (next_exps(lambda, threshED));
					cTr++;
				}else{
					*(tIO + iTr) = (next_exps(lambda, threshED)*10 + 2);
					iTr++;
				}
		}
		*(tArray+0) = aBD;
		*(tArray+1) = tCPU;
		*(tArray+2) = tIO;
		*(data+i) = tArray;
	}
	return data;
}

int*** deepCopy(int*** arr, int simN){
	int burstN;
	int aT;
	int*** data = calloc(simN, sizeof(int**));
	for (int i = 0; i < simN; ++i)
	{
		int* aBDC = calloc(3, sizeof(int));
		int temp = arr[i][0][1];
		burstN = temp;
		aT = arr[i][0][0];
		
		*(aBDC + 0) = aT;
		*(aBDC + 1) = burstN;
		*(aBDC + 2) = 0;

		int* tCPUC = calloc(burstN, sizeof(int));
		for (int b = 0; b < burstN; ++b)
		{
			tCPUC[b] = arr[i][1][b];
		}

		int* tIOC = calloc((burstN-1), sizeof(int));
		for (int b = 0; b < burstN - 1; ++b)
		{
			tIOC[b] = arr[i][2][b];
		}

		int** tArrayC = calloc(3, sizeof(int*));

		*(tArrayC+0) = aBDC;
		*(tArrayC+1) = tCPUC;
		*(tArrayC+2) = tIOC;
		*(data+i) = tArrayC;
	}

	return data;

}

void printData(int*** data, int simN){
	for (int i = 0; i < simN; ++i)
	{
		printf("%d, %d, %d \n", data[i][0][0], data[i][0][1], data[i][0][2]);
		int num = data[i][0][1];

		for (int b = 0; b < num; ++b)
		{
			printf(" %d\n", data[i][1][b]);
		}
		printf("IO\n");
		for (int b = 0; b < num - 1; ++b)
		{
			printf(" %d\n", data[i][2][b]);
		}
	}
}

void freeData(int*** data, int simN){
	for (int i = 0; i < simN; ++i)
	{
		free(data[i][0]);
		free(data[i][1]);
		free(data[i][2]);
		free(data[i]);
	}
	free(data);
}

void printDataCompare(int*** data1, int*** data2, int simN){
	for (int i = 0; i < simN; ++i)
	{
		printf("%d, %d, %d \n", data1[i][0][0], data1[i][0][1], data1[i][0][2]);
		printf("%d, %d, %d \n", data2[i][0][0], data2[i][0][1], data2[i][0][2]);
		int num = data1[i][0][1];

		for (int b = 0; b < num; ++b)
		{
			printf(" %d\n", data1[i][1][b]);
			printf(" %d\n", data2[i][1][b]);
		}
		for (int b = 0; b < num - 1; ++b)
		{
			printf(" %d\n", data1[i][2][b]);
			printf(" %d\n", data2[i][2][b]);
		}
	}
}

char getProcessName(int process){
	char ALPHABET[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
	char ret = ALPHABET[process];
	return ret;
}

int computeTau(int preTau, int actual, double alphC){
	int ret = ceil((preTau * (1-alphC)) + (alphC * actual));
	return ret;
}

void sortbyArrival(queue_item** hiddenQueue, int simN){
	queue_item* temp;
	queue_item* temp2;
	for (int i = 0; i < simN; ++i)
	{
		int min = INT_MAX;
		int minIn = INT_MAX;
		for (int b = i; b < simN; ++b)
		{
			if (((*hiddenQueue[b]).arrivalTime) <= min)
			{
				if ((*hiddenQueue[b]).arrivalTime == min)
				{
					if ((*hiddenQueue[b]).ID < (*hiddenQueue[minIn]).ID)
					{
						min = (*hiddenQueue[b]).arrivalTime;
						minIn = b;
					}
					else{
						continue;
					}
				}
				else{
					min = (*hiddenQueue[b]).arrivalTime;
					minIn = b;
				}
			}
		}
		temp = hiddenQueue[minIn];
		temp2 = hiddenQueue[i];
		hiddenQueue[minIn] = temp2;
		hiddenQueue[i] = temp;
	}
}

queue_item** createQueue(int*** data, int simN, int tau){
	queue_item** hiddenQueue = calloc(simN, sizeof(queue_item*));
	for (int i = 0; i < simN; ++i)
	 {
	 	queue_item* item = malloc(sizeof(queue_item));
	 	(*item).wait = 0;

	 	(*item).procNum = 0;
	 	(*item).IONum = 0;
	 	(*item).waitSize = 0;
	 	(*item).turnSize = 0;
	 	(*item).arrivalTimeSize = 1;
	 	(*item).ioEndTime = -1;
	 	(*item).burstEndTime = -1;
	 	(*item).tau = tau;
	 	(*item).burstN = data[i][0][1];
	 	(*item).suspended = 0;

	 	(*item).wait = calloc((*item).burstN, sizeof(int));
		(*item).turnaround = calloc((*item).burstN, sizeof(int));
		(*item).arrivalTimes = calloc((*item).burstN, sizeof(int));

		(*item).ID = i;
		(*item).arrivalTime = data[i][0][0];
		(*item).arrivalTimes[0] = data[i][0][0];
		

		

		(*item).burst = data[i][1];
		(*item).IO = data[i][2];

	 	hiddenQueue[i] = item;
	 } 

	 return hiddenQueue;
}

void freeQueue(queue_item** queue, int simN){
	for (int i = 0; i < simN; ++i)
	{
		free((*queue[i]).wait);
		free((*queue[i]).turnaround);
		free((*queue[i]).arrivalTimes);
		free(queue[i]);
	}
	free(queue);
}

void printHeader(int*** data, int simN, double lambda ){
	int tau = ceil(1/lambda);
	for (int i = 0; i < simN; ++i)
	{
		printf("Process %c (arrival time %d ms) %d CPU bursts (tau %dms)\n", getProcessName(i), data[i][0][0], data[i][0][1], tau);
	}
}

void printQueue(queue_item** Queue, int queueSize){
	if (queueSize == 0)
	{
		printf(" empty]\n");
		return;
	}
	for (int i = 0; i < queueSize; ++i)
	{
		printf("%c", getProcessName((*Queue[i]).ID));
	}

	printf("]\n");
}

void addToQueue(queue_item** Queue, queue_item* item, int* queueSize){
	Queue[(*queueSize)] = item;
	*queueSize += 1;
}

void sortQueueSJF(queue_item** Queue, int queueSize){
	queue_item* temp;
	queue_item* temp2;
	for (int i = 0; i < queueSize; ++i)
	{
		int min = INT_MAX;
		int minIn = INT_MAX;
		for (int b = i; b < queueSize; ++b)
		{
			if (((*Queue[b]).tau) <= min)
			{
				if ((*Queue[b]).tau == min)
				{
					if ((*Queue[b]).ID < (*Queue[minIn]).ID)
					{
						min = (*Queue[b]).tau;
						minIn = b;
					}
					else{
						continue;
					}
				}
				else{
					min = (*Queue[b]).tau;
					minIn = b;
				}
			}
		}
		temp = Queue[minIn];
		temp2 = Queue[i];
		Queue[minIn] = temp2;
		Queue[i] = temp;
	}
}

void sortIO(queue_item** Queue, int queueSize){
	queue_item* temp;
	queue_item* temp2;
	for (int i = 0; i < queueSize; ++i)
	{
		int min = INT_MAX;
		int minIn = INT_MAX;
		for (int b = i; b < queueSize; ++b)
		{
			if (((*Queue[b]).ioEndTime) <= min)
			{
				if ((*Queue[b]).ioEndTime == min)
				{
					if ((*Queue[b]).ID < (*Queue[minIn]).ID)
					{
						min = (*Queue[b]).ioEndTime;
						minIn = b;
					}
					else{
						continue;
					}
				}
				else{
					min = (*Queue[b]).ioEndTime;
					minIn = b;
				}
			}
		}
		temp = Queue[minIn];
		temp2 = Queue[i];
		Queue[minIn] = temp2;
		Queue[i] = temp;
	}
}

void popFront(queue_item** Queue, int* queueSize){
	for (int i = 1; i < *queueSize; ++i)
	{
		Queue[i-1] = Queue[i];
		Queue[i] = NULL;
	}
	*queueSize -= 1;
}


double* sjf(int*** data, double conSwitch, double lambda, double alphC, int simN){
	int tau = ceil(1/lambda);
	int time = 0;
	int queueSize = 0;
	int hiddenQueueIndex = 0;
	int IOSize = 0;
	int IOIndex = 0;
	int burstSize = 0;
	double contextSwitches = 0;
	double premtions = 0;

	int contextSwitch = 0;

	queue_item** Queue = calloc(simN, sizeof(queue_item));
	queue_item** IO = calloc(simN, sizeof(queue_item));
	queue_item** burst = calloc(1, sizeof(queue_item));
	queue_item* switching;

	queue_item** hiddenQueue = createQueue(data, simN, tau);

	int onDeck = 0;
	
	sortbyArrival(hiddenQueue, simN);

	printf("time %dms: Simulator started for SJF [Q empty]\n", time);

	while(1){
		if(hiddenQueueIndex == simN && IOSize == 0 && burstSize == 0 && queueSize == 0 && contextSwitch == 0 && onDeck == 0){
			printf("time %dms: Simulator ended for SJF [Q empty]\n", time);
			break;
		}

		if (onDeck > 0 && contextSwitch == 0)
		{
			if (onDeck == 1)
			{
				(*Queue[0]).wait[(*Queue[0]).waitSize] = time - conSwitch/2;
				(*Queue[0]).waitSize += 1;
				burst[0] = Queue[0];
				contextSwitches += 1;
				popFront(Queue, &queueSize);
			}
			else{
				burst[0] = switching;
			}

			burstSize = 1;
			onDeck = 0;
			(*burst[0]).burstEndTime = time + (*burst[0]).burst[(*burst[0]).procNum];
			(*burst[0]).prevActual = (*burst[0]).burst[(*burst[0]).procNum];
			

			if (time < 1000)
			{
				printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst [Q ", time, getProcessName((*burst[0]).ID), (*burst[0]).tau, (*burst[0]).burst[(*burst[0]).procNum]);
				printQueue(Queue, queueSize);
			}
		}


		//End Burst
		if(burstSize != 0 && (*burst[0]).burstEndTime == time){
			//terminate burst
			if ((*burst[0]).procNum == ((*burst[0]).burstN - 1))
			{
				printf("time %dms: Process %c terminated [Q ", time, getProcessName((*burst[0]).ID));
				printQueue(Queue, queueSize);
				contextSwitch += conSwitch/2;
				(*burst[0]).turnaround[(*burst[0]).turnSize] = time + conSwitch/2;
				(*burst[0]).turnSize += 1;
			}
			//completed burst
			else{
				(*burst[0]).procNum += 1;
				contextSwitch += conSwitch/2;
				(*burst[0]).ioEndTime = time + (*burst[0]).IO[(*burst[0]).IONum];
				int tempPre = (*burst[0]).tau;
				(*burst[0]).tau = computeTau(tempPre, (*burst[0]).prevActual, alphC);

				addToQueue(IO, burst[0], &IOSize);
				sortIO(IO, IOSize);
				(*burst[0]).turnaround[(*burst[0]).turnSize] = time + conSwitch/2;
				(*burst[0]).turnSize += 1;
				if (time < 1000)
				{
					printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d bursts to go [Q ", time, getProcessName((*burst[0]).ID), tempPre, (((*burst[0]).burstN) - (*burst[0]).procNum));
					printQueue(Queue, queueSize);

					printf("time %dms: Recalculated tau from %d to %dms for process %c [Q ", time, tempPre, (*burst[0]).tau, getProcessName((*burst[0]).ID));
					printQueue(Queue, queueSize);
					
					printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q ", time, getProcessName((*burst[0]).ID), (*burst[0]).ioEndTime);
					printQueue(Queue, queueSize);
				}
			}
			burst[0] = NULL;
			burstSize = 0;
		}


		int presize = queueSize;

		//io
		int count = 0;
		while(IOSize > 0 && IOIndex < IOSize && (*IO[IOIndex]).ioEndTime == time){
			count+= 1;
			addToQueue(Queue, IO[IOIndex], &queueSize);
			sortQueueSJF(Queue, queueSize);
			(*IO[IOIndex]).IONum += 1;
			(*IO[IOIndex]).arrivalTimes[(*IO[IOIndex]).arrivalTimeSize] = time;
			(*IO[IOIndex]).arrivalTimeSize += 1;
			if (time < 1000)
			{
				printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue [Q ", time, getProcessName((*IO[IOIndex]).ID), (*IO[IOIndex]).tau);
				printQueue(Queue, queueSize);
			}
			IOIndex += 1;
		}
		IOIndex = 0;
		for (int i = 0; i < count; ++i)
		{
			popFront(IO, &IOSize);
		}
		
		//add process queue
		while(hiddenQueueIndex < simN && (*hiddenQueue[hiddenQueueIndex]).arrivalTime == time)
		{
			addToQueue(Queue, hiddenQueue[hiddenQueueIndex], &queueSize);
			sortQueueSJF(Queue, queueSize);
			if (time < 1000)
			{
				printf("time %dms: Process %c (tau %dms) arrived; added to ready queue [Q ", time, getProcessName((*hiddenQueue[hiddenQueueIndex]).ID), (*hiddenQueue[hiddenQueueIndex]).tau);
				printQueue(Queue, queueSize);
			}

			hiddenQueueIndex +=1;
		}

		//printf("%d, %p, %d\n", burstSize, onDeck, queueSize);

		if (burstSize == 0 && !onDeck && queueSize > 0)
		{
			contextSwitch += conSwitch/2;
			
			if (presize == 0)
			{
				onDeck = 2;

				(*Queue[0]).wait[(*Queue[0]).waitSize] = time;
				(*Queue[0]).waitSize += 1;
				switching = Queue[0];
				contextSwitches += 1;
				popFront(Queue, &queueSize);
			}
			else{
				onDeck = 1;
			}
		}

		time+= 1;
		if (contextSwitch !=0)
		{
			contextSwitch -= 1;
		}
		
	}
	for (int i = 0; i < simN; ++i)
	{
		int num = (*hiddenQueue[i]).turnSize;
		for (int b = 0; b < num; ++b)
		{
			(*hiddenQueue[i]).turnaround[b] = (*hiddenQueue[i]).turnaround[b] - (*hiddenQueue[i]).arrivalTimes[b];
			(*hiddenQueue[i]).wait[b] = (*hiddenQueue[i]).wait[b] - (*hiddenQueue[i]).arrivalTimes[b];
		}
	}
	int div = 0;
	int totalTa = 0;
	int totalW = 0;
	int totalBurst = 0;
	for (int i = 0; i < simN; ++i)
	{
		int num = (*hiddenQueue[i]).turnSize;

		div += (*hiddenQueue[i]).burstN;
		for (int b = 0; b < num; ++b)
		{
			totalTa += (*hiddenQueue[i]).turnaround[b];
			totalW += (*hiddenQueue[i]).wait[b];
			totalBurst += (*hiddenQueue[i]).burst[b];
		}	
	}

	double avgTurnaround = (double)totalTa/(double)div;
	double avgWait = ((double)totalW/(double)div);
	double avgBurst = (double)totalBurst/(double)div;
	double cpu = 100 * ((double)totalBurst/(double)time);

	double * ret = calloc(6, sizeof(double));
	ret[0] = avgBurst;
	ret[1] = avgWait;
	ret[2] = avgTurnaround;
	ret[3] = contextSwitches;
	ret[4] = premtions;
	ret[5] = cpu;
	freeQueue(hiddenQueue, simN);
	free(Queue);
	free(IO);
	free(burst);

	return ret;
}

void sortQueueSRT(queue_item** Queue, int queueSize){
	queue_item* temp;
	queue_item* temp2;
	for (int i = 0; i < queueSize; ++i)
	{
		int min = INT_MAX;
		int minIn = INT_MAX;
		for (int b = i; b < queueSize; ++b)
		{
			if ((*Queue[b]).suspended != 0)
			{
				printf("Suspended %d %d %d %d\n ", i , b, min, (*Queue[b]).suspended);
				if (((*Queue[b]).suspended) <= min)
				{
					printf("Suspended\n");
					if ((*Queue[b]).suspended == min)
					{
						printf("Suspended\n");
						if ((*Queue[b]).ID < (*Queue[minIn]).ID)
						{
							printf("Suspended\n");
							min = (*Queue[b]).suspended;
							minIn = b;
						}
						else{
							continue;
						}
					}
					else{
						printf("Suspended Swaped\n");
						min = (*Queue[b]).suspended;
						minIn = b;
					}
				}
			}
			else if(((*Queue[b]).tau) <= min)
			{
				if ((*Queue[b]).tau == min)
				{
					if ((*Queue[b]).ID < (*Queue[minIn]).ID)
					{
						min = (*Queue[b]).tau;
						minIn = b;
					}
					else{
						continue;
					}
				}
				else{
					min = (*Queue[b]).tau;
					minIn = b;
				}
			}
		}
		temp = Queue[minIn];
		temp2 = Queue[i];
		Queue[minIn] = temp2;
		Queue[i] = temp;
	}
}


double* srt(int*** data, double conSwitch, double lambda, double alphC, int simN){
	int tau = ceil(1/lambda);
	int time = 0;
	int queueSize = 0;
	int hiddenQueueIndex = 0;
	int IOSize = 0;
	int IOIndex = 0;
	int burstSize = 0;
	double contextSwitches = 0;
	double premtions = 0;

	int contextSwitch = 0;

	queue_item** Queue = calloc(simN, sizeof(queue_item));
	queue_item** IO = calloc(simN, sizeof(queue_item));
	queue_item** burst = calloc(1, sizeof(queue_item));
	queue_item* switching;

	queue_item** hiddenQueue = createQueue(data, simN, tau);

	int onDeck = 0;
	
	sortbyArrival(hiddenQueue, simN);

	printf("time %dms: Simulator started for SJF [Q empty]\n", time);

	while(1){
		if(hiddenQueueIndex == simN && IOSize == 0 && burstSize == 0 && queueSize == 0 && contextSwitch == 0 && onDeck == 0){
			printf("time %dms: Simulator ended for SJF [Q empty]\n", time);
			break;
		}

		if (onDeck > 0 && contextSwitch == 0)
		{
			if (onDeck ==  1)
			{
				(*Queue[0]).wait[(*Queue[0]).waitSize] = time + conSwitch/2;
				(*Queue[0]).waitSize += 1;
				burst[0] = Queue[0];
				contextSwitches += 1;
				popFront(Queue, &queueSize);
			}
			else if(onDeck ==  3){
				(*switching).wait[(*switching).waitSize] = time - conSwitch;
				(*switching).waitSize += 1;
				burst[0] = switching;
			}
			else{
				burst[0] = switching;
			}

			burstSize = 1;
			onDeck = 0;
			if ((*burst[0]).suspended != 0)
			{
				(*burst[0]).burstEndTime = time + (*burst[0]).suspended - conSwitch;
				if (time < 1000000)
				{
					printf("time %dms: Process %c (tau %dms) started using the CPU for remaining %dms of %dms burst [Q ", time, getProcessName((*burst[0]).ID), (*burst[0]).tau,(*burst[0]).suspended, (*burst[0]).burst[(*burst[0]).procNum]);
					printQueue(Queue, queueSize);
				}
				//(*burst[0]).suspended = 0;
			}
			else{
				(*burst[0]).burstEndTime = time + (*burst[0]).burst[(*burst[0]).procNum];
				if (time < 1000000)
				{
					printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst [Q ", time, getProcessName((*burst[0]).ID), (*burst[0]).tau, (*burst[0]).burst[(*burst[0]).procNum]);
					printQueue(Queue, queueSize);
				}
			}
			
			(*burst[0]).prevActual = (*burst[0]).burst[(*burst[0]).procNum];
			


		}


		//End Burst
		if(burstSize != 0 && (*burst[0]).burstEndTime == time){
			//terminate burst
			if ((*burst[0]).procNum == ((*burst[0]).burstN - 1))
			{
				printf("time %dms: Process %c terminated [Q ", time, getProcessName((*burst[0]).ID));
				printQueue(Queue, queueSize);
				contextSwitch += conSwitch/2;
				(*burst[0]).turnaround[(*burst[0]).turnSize] = time + conSwitch/2;
				(*burst[0]).turnSize += 1;
			}
			//completed burst
			else{
				(*burst[0]).procNum += 1;
				contextSwitch+= conSwitch/2;
				(*burst[0]).ioEndTime = time + (*burst[0]).IO[(*burst[0]).IONum];
				int tempPre = (*burst[0]).tau;
				(*burst[0]).tau = computeTau(tempPre, (*burst[0]).prevActual, alphC);

				addToQueue(IO, burst[0], &IOSize);
				sortIO(IO, IOSize);
				(*burst[0]).turnaround[(*burst[0]).turnSize] = time + conSwitch/2;
				(*burst[0]).turnSize += 1;
				if (time < 1000000)
				{
					printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d bursts to go [Q ", time, getProcessName((*burst[0]).ID), tempPre, (((*burst[0]).burstN) - (*burst[0]).procNum));
					printQueue(Queue, queueSize);

					printf("time %dms: Recalculated tau from %d to %dms for process %c [Q ", time, tempPre, (*burst[0]).tau, getProcessName((*burst[0]).ID));
					printQueue(Queue, queueSize);
					
					printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q ", time, getProcessName((*burst[0]).ID), (*burst[0]).ioEndTime);
					printQueue(Queue, queueSize);
				}
			}
			(*burst[0]).suspended = 0;
			burst[0] = NULL;
			burstSize = 0;

		}


		int presize = queueSize;



		//io
		int count = 0;
		while(IOSize > 0 && IOIndex < IOSize && (*IO[IOIndex]).ioEndTime == time){
			count+= 1;
			addToQueue(Queue, IO[IOIndex], &queueSize);
			sortQueueSJF(Queue, queueSize);
			(*IO[IOIndex]).IONum += 1;
			(*IO[IOIndex]).arrivalTimes[(*IO[IOIndex]).arrivalTimeSize] = time;
			(*IO[IOIndex]).arrivalTimeSize += 1;
			
			if (burstSize != 0 && onDeck == 0)
			{
				double compare;
				//if ((*burst[0]).suspended != 0)
				//{
				//	compare = (*burst[0]).tau - ((*burst[0]).burst[(*burst[0]).procNum] - ((*burst[0]).suspended - (time - ((*burst[0]).wait[(*burst[0]).waitSize - 1] - conSwitch/2))));
				//	printf("sus %d - (%d - (%d - (%d -(%d - %f)))) = %f\n",(*burst[0]).tau, (*burst[0]).burst[(*burst[0]).procNum], (*burst[0]).suspended, time, (*burst[0]).wait[(*burst[0]).waitSize - 1], conSwitch/2,compare);
				//}

				//else{
					compare = ((*burst[0]).tau - (time - ((*burst[0]).wait[(*burst[0]).waitSize - 1] - conSwitch/2))); 
					printf("thing %d - (%d - %f ) = %f \n", (*burst[0]).tau , time, (*burst[0]).wait[(*burst[0]).waitSize - 1] - conSwitch/2, compare);
				//}
				if((*IO[IOIndex]).tau < compare){
					if (time < 1000000)
					{
						printf("time %dms: Process %c (tau %dms) completed I/O; preempting %c [Q ", time, getProcessName((*IO[IOIndex]).ID), (*IO[IOIndex]).tau, getProcessName((*burst[0]).ID));
						printQueue(Queue, queueSize);
					}

					switching = Queue[0];
					popFront(Queue, &queueSize);
					if ((*burst[0]).suspended == 0)
					{
						(*burst[0]).suspended = (*burst[0]).burst[(*burst[0]).procNum] - (time - (*burst[0]).wait[(*burst[0]).waitSize - 1] - conSwitch/2);
					}
					else{
						//printf("%d %f\n", (*burst[0]).suspended, (*burst[0]).wait[(*burst[0]).waitSize - 1] - conSwitch/2);
						(*burst[0]).suspended = (*burst[0]).suspended - (time - (*burst[0]).wait[(*burst[0]).waitSize - 1] - conSwitch/2);
					}
					
					printf("%d\n", (*burst[0]).suspended);
					addToQueue(Queue, burst[0], &queueSize);
					printQueue(Queue, queueSize);
					sortQueueSJF(Queue, queueSize);
					printQueue(Queue, queueSize);
					(*burst[0]).waitSize -= 1;
					burst[0] = NULL;
					burstSize = 0;
					onDeck = 3;

					contextSwitch += conSwitch;
				}
				else if (time < 1000000)
				{
					printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue [Q ", time, getProcessName((*IO[IOIndex]).ID), (*IO[IOIndex]).tau);
					printQueue(Queue, queueSize);
				}
			}


			
			IOIndex += 1;
		}
		IOIndex = 0;
		for (int i = 0; i < count; ++i)
		{
			popFront(IO, &IOSize);
		}
		
		//add process queue
		while(hiddenQueueIndex < simN && (*hiddenQueue[hiddenQueueIndex]).arrivalTime == time)
		{
			addToQueue(Queue, hiddenQueue[hiddenQueueIndex], &queueSize);
			sortQueueSJF(Queue, queueSize);
			if (time < 1000000)
			{
				printf("time %dms: Process %c (tau %dms) arrived; added to ready queue [Q ", time, getProcessName((*hiddenQueue[hiddenQueueIndex]).ID), (*hiddenQueue[hiddenQueueIndex]).tau);
				printQueue(Queue, queueSize);
			}

			hiddenQueueIndex +=1;
		}

		//printf("%d, %p, %d\n", burstSize, onDeck, queueSize);

		if (burstSize == 0 && onDeck == 0 && queueSize > 0)
		{
			contextSwitch += 0.5 * conSwitch;
			
			if (presize == 0)
			{
				onDeck = 2;
				(*Queue[0]).wait[(*Queue[0]).waitSize] = time - conSwitch;
				(*Queue[0]).waitSize += 1;
				switching = Queue[0];
				contextSwitches += 1;
				popFront(Queue, &queueSize);
			}
			else{
				onDeck = 1;
			}
		}

		time+= 1;
		if (contextSwitch !=0)
		{
			contextSwitch -= 1;
		}
		
	}
	for (int i = 0; i < simN; ++i)
	{
		int num = (*hiddenQueue[i]).turnSize;
		for (int b = 0; b < num; ++b)
		{
			(*hiddenQueue[i]).turnaround[b] = (*hiddenQueue[i]).turnaround[b] - (*hiddenQueue[i]).arrivalTimes[b];
			(*hiddenQueue[i]).wait[b] = (*hiddenQueue[i]).wait[b] - (*hiddenQueue[i]).arrivalTimes[b];
		}
	}
	int div = 0;
	int totalTa = 0;
	int totalW = 0;
	int totalBurst = 0;
	for (int i = 0; i < simN; ++i)
	{
		int num = (*hiddenQueue[i]).turnSize;

		div += (*hiddenQueue[i]).burstN;
		for (int b = 0; b < num; ++b)
		{
			totalTa += (*hiddenQueue[i]).turnaround[b];
			totalW += (*hiddenQueue[i]).wait[b];
			totalBurst += (*hiddenQueue[i]).burst[b];
		}	
	}

	double avgTurnaround = (double)totalTa/(double)div;
	double avgWait = ((double)totalW/(double)div);
	double avgBurst = (double)totalBurst/(double)div;
	double cpu = 100 * ((double)totalBurst/(double)time);

	double * ret = calloc(6, sizeof(double));
	ret[0] = avgBurst;
	ret[1] = avgWait;
	ret[2] = avgTurnaround;
	ret[3] = contextSwitches;
	ret[4] = premtions;
	ret[5] = cpu;
	freeQueue(hiddenQueue, simN);
	free(Queue);
	free(IO);
	free(burst);

	return ret;
}


void writeFile(FILE *fp, double* data){
	fprintf(fp, "-- average CPU burst time: %.3f ms\n", data[0]);
	fprintf(fp, "-- average wait time: %.3f ms\n", data[1]);
	fprintf(fp, "-- average turnaround time: %.3f ms\n", data[2]);
	fprintf(fp, "-- total number of context switches: %.0f\n", data[3]);
	fprintf(fp, "-- total number of preemptions: %.0f\n", data[4]);
	fprintf(fp, "-- CPU utilization: %.3f%%\n", data[5]);
}


/*
next_exp index: data[][][]
data[] determines which process: data[0] = A, data[25] = Z
data[][] determines data type: data[][0] = arrival time and cpu busts IN THAT ORDER,
Third thing is to track which cpu processes are done
data[][1] = CPU burst times IN ORDER, data[][2] = I/O add times IN ORDER
data[][][] refers to each indivisual value
*/


int main(int argc, char * argv[]){
	int simN = atoi(argv[1]);
	int seedR = atoi(argv[2]);
	double lambda = strtod(argv[3], NULL);
	int threshED = atoi(argv[4]);
	int conSwitch = atoi(argv[5]);
	double alphC = strtod(argv[6], NULL);
	int timeSlice = atoi(argv[7]);

	char* filename = malloc(11);
	strcpy(filename, "simout.txt");	
	FILE *fp = fopen(filename, "w");
	free(filename);
	double * printvals;

	srand48(seedR);

	fprintf(fp, "Algorithm FCFS\n");

	//FCFS
	int*** FCFSD = next_exp(lambda, simN, threshED);
	printHeader(FCFSD, simN, lambda);
	
	freeData(FCFSD, simN);

	srand48(seedR);
	fprintf(fp, "Algorithm SJF\n");
	//SJF;
	int*** SJFD = next_exp(lambda, simN, threshED);
	//printvals = sjf(SJFD, conSwitch, lambda, alphC, simN);
	//writeFile(fp, printvals);
	//free(printvals);

	freeData(SJFD, simN);
	

	srand48(seedR);
	fprintf(fp, "Algorithm SRT\n");
	//SRT;
	int*** SRTD = next_exp(lambda, simN, threshED);
	printvals = srt(SRTD, conSwitch, lambda, alphC, simN);
	writeFile(fp, printvals);
	free(printvals);

	freeData(SRTD, simN);

	srand48(seedR);
	fprintf(fp, "Algorithm RR\n");
	//RR;
	int*** RRD = next_exp(lambda, simN, threshED);
	freeData(RRD, simN);
	fclose(fp);
	return 0;
}