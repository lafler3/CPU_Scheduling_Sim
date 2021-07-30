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
	int wait;
	int turnaround;

	int ID;
	int arrivalTime;
	int burstN;
	int tau;
	int prevActual;

	int procNum;
	int IONum;
	int* burst;
	int* IO;

	int ioEndTime;
	int burstEndTime;
	int suspended;
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
	 	(*item).ioEndTime = -1;
	 	(*item).burstEndTime = -1;
	 	(*item).tau = tau;

		(*item).ID = i;
		(*item).arrivalTime = data[i][0][0];
		(*item).burstN = data[i][0][1];

		(*item).burst = data[i][1];
		(*item).IO = data[i][2];

	 	hiddenQueue[i] = item;
	 } 

	 return hiddenQueue;
}

void freeQueue(queue_item** queue, int simN){
	for (int i = 0; i < simN; ++i)
	{
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


void sjf(int*** data, double conSwitch, double lambda, double alphC, int simN){
	int tau = ceil(1/lambda);
	int time = 0;
	int queueSize = 0;
	int hiddenQueueIndex = 0;
	int IOSize = 0;
	int IOIndex = 0;
	int burstSize = 0;

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
			printf("time %dms: Simulator ended for SJF [Q empty]", time);
			break;
		}

		if (onDeck > 0 && contextSwitch == 0)
		{
			if (onDeck == 1)
			{
				burst[0] = Queue[0];
				popFront(Queue, &queueSize);
			}
			else{
				burst[0] = switching;
			}

			burstSize = 1;
			onDeck = 0;
			(*burst[0]).burstEndTime = time + (*burst[0]).burst[(*burst[0]).procNum];
			(*burst[0]).prevActual = (*burst[0]).burst[(*burst[0]).procNum];
			
			if (time < 1000000)
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
			if (time < 1000000)
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
			if (time < 1000000)
			{
				printf("time %dms: Process %c (tau %dms) arrived; added to ready queue [Q ", time, getProcessName((*hiddenQueue[hiddenQueueIndex]).ID), (*hiddenQueue[hiddenQueueIndex]).tau);
				printQueue(Queue, queueSize);
			}

			hiddenQueueIndex +=1;
		}

		//printf("%d, %p, %d\n", burstSize, onDeck, queueSize);

		if (burstSize == 0 && !onDeck && queueSize > 0)
		{
			contextSwitch += 2;
			
			if (presize == 0)
			{
				onDeck = 2;
				switching = Queue[0];
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

	freeQueue(hiddenQueue, simN);
	free(Queue);
	free(IO);
	free(burst);
}

int get_smallest_arrival(int*** data, int simN, int* KillT, int KillL){
	int beI = -1;
	int best = data[0][0][0];
	for(int i = 0; i< simN; i++){
		int inK = 0;
		for(int j = 0; j<KillL; j++){
			if(KillT[j] == i){
				inK = 1;
			}
		}
		if(data[i][0][0] < best && inK == 0){
			best = data[i][0][0];
			beI = i;
		}
	}
	return beI;
}


void rr(int*** data, int simN, int timeSlice, int RRC){
	/*
		while que is not empty and there are no processes
		if que is not empty and there is no activity, 
		run a process (the first)

	*/
	if(RRC == 1){
		printf("time 0ms: Simulator started for RR [Q empty]\n");
	}else{
		printf("time 0ms: Simulator started for FCFS [Q empty]\n");
	}
	if(simN == 0){
		exit(EXIT_SUCCESS);
	}
	char* Queue = calloc(simN, sizeof(char));
	int* pQueue = calloc(simN, sizeof(int)); 
	int* KillL = calloc(simN, sizeof(int));
	int qT = 0;
	int killT = 0;
	int Crunning = 0;
	int Ccount = 0;
	int time = 0;
	int CurrentP = -1;

	int* IOn = calloc(simN, sizeof(int));
	int* IOv = calloc(simN, sizeof(int));
	for(int i = 0; i<simN; i++){
		*(IOn+i) = 0;
	}
//make 2 arrays for io - one to track if on, another to check time
//when a context switch happens, the time left is stored at data[i][0][3]
//then return to back of line. make sure to check that variable
	while(qT != 0 || killT != simN || Crunning == 1){
		int smallAr = get_smallest_arrival(data, simN, killT, KillL);
		char fP = getProcessName(smallAr);
		if(time == data[smallAr][0][0]){
			*(KillL + killT) = smallAr;
			killT++;
			*(Queue + qT)  = fP;
			*(pQueue + qT)  = smallAr;
			qT++;
			printf("time %dms: Process %c arrived; added to ready queue [Q %s]\n", time, fP, Queue);
		}

		//checking io
		for(int i = 0; i<simN; i++){
			if((IOv[i]) == time && IOn[i] == 1){
				*(Queue + qT) = getProcessName(i);
				*(pQueue + qT) = i;
				qT++;
				printf("time %dms: Process %c completed I/O; added to ready queue [Q %s]\n", time, getProcessName(i), Queue);
				*(IOn + i) = 0;
			}
		}

		if(Crunning == 0 && qT != 0){
			Ccount = time;
			//make an exception for emptying que
			char hold = Queue[0];
			int holdP = pQueue[0];
			qT--;
			for(int j = 0; j<qT; j++){
				*(Queue + j) = *(Queue + (j+1));
				*(pQueue + j) = *(pQueue + (j+1));
			}
			*(Queue + (qT+1)) = '\0';
			if(qT == 0){
				printf("time %dms: Process %c started using the CPU for %dms burst [Q empty]\n", time, hold, data[holdP][1][data[holdP][0][2]]);
			}else{
				printf("time %dms: Process %c started using the CPU for %dms burst [Q %s]\n", time, hold, data[holdP][1][data[holdP][0][2]], Queue);
			}
			Ccount = Ccount + data[holdP][0][1];
			CurrentP = holdP;
			Crunning = 1;

			int** tt1 = data[holdP];
			int* tt2 = tt1[0];
			*(tt2 + 2) = *(tt2 + 2) + 1;

		}
		if(Crunning == 1 && time == Ccount){
			//exception for empty que


			if(qT == 0){
				printf("time %dms: Process %c completed a CPU burst; %d bursts to go [Q empty]\n", time, getProcessName(CurrentP), (data[CurrentP][0][1]-data[CurrentP][0][2]));
			}else{
				printf("time %dms: Process %c completed a CPU burst; %d bursts to go [Q %s]\n", time, getProcessName(CurrentP), (data[CurrentP][0][1]-data[CurrentP][0][2]), Queue);
			}
			//start IO
			Ccount = 0;
			Crunning = 0;

			if((data[CurrentP][0][1]-data[CurrentP][0][2]) != 1){
				*(IOn + CurrentP) = 1;
				*(IOv + CurrentP) =  data[CurrentP][2][data[CurrentP][0][2]] + time;
				printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q %s]\n", time, Queue[CurrentP], IOv[CurrentP], Queue);
			}
		}
		time++;
	}
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

	srand48(seedR);
	//FCFS
	int*** FCFSD = next_exp(lambda, simN, threshED);
	printHeader(FCFSD, simN, lambda);

	
	freeData(FCFSD, simN);
	srand48(seedR);
	//SJF;
	int*** SJFD = next_exp(lambda, simN, threshED);
	sjf(SJFD, conSwitch, lambda, alphC, simN);
	freeData(SJFD, simN);
	

	srand48(seedR);
	//SRT;
	int*** SRTD = next_exp(lambda, simN, threshED);
	freeData(SRTD, simN);

	srand48(seedR);
	//RR;
	int*** RRD = next_exp(lambda, simN, threshED);
	freeData(RRD, simN);

	rr(FCFSD, simN, timeSlice, 0);

	return 0;
}