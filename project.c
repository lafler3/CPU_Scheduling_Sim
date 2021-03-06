//Project File Opsys
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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

	int procNum;
	int IONum;
	int* burst;
	int* IO;

	int suspended;
}
queue_item;


int next_exps(double lambda, int threshED){
	int aT = threshED+1;
	while(aT > threshED){
		double r = drand48();
		aT=floor(-log(r)/lambda);
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
					*(tCPU + cTr) = (next_exp(lambda, threshED)+1);
					cTr++;
				}else{
					*(tIO + iTr) = (next_exp(lambda, threshED)*10)+12;
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

int avg(double total, double processes){
	int ret = ceil(total/processes);
	return ret;
}

void sortbyArrival(queue_item** hiddenQueue, int simN){
	queue_item* temp;
	queue_item* temp2;
	for (int i = 0; i < simN; ++i)
	{
		int min = 99999;
		int minIn = 999999;
		for (int b = i; b < simN; ++b)
		{
			if ((*hiddenQueue[b]).arrivalTime < min)
			{
				min = (*hiddenQueue[b]).arrivalTime;
				minIn = b;
			}
		}
		temp = hiddenQueue[minIn];
		temp2 = hiddenQueue[i];
		hiddenQueue[minIn] = temp2;
		hiddenQueue[i] = temp;
	}
}

queue_item** createQueue(int*** data, int simN){
	queue_item** hiddenQueue = calloc(simN, sizeof(queue_item*));
	for (int i = 0; i < simN; ++i)
	 {
	 	queue_item* item = malloc(sizeof(queue_item));
	 	(*item).procNum = 0;
	 	(*item).wait = 0;
	 	(*item).IONum = 0;

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

void sjf(int*** data, double conSwitch, double alphC, int simN){
	double total = ceil(1/alphC);
	double processes = 1;
	int tau = avg(total, processes);
	
	int queueSize = 0;
	queue_item** Queue = calloc(simN, sizeof(queue_item));

	queue_item** hiddenQueue = createQueue(data, simN);
	sortbyArrival(hiddenQueue, simN);


	freeQueue(hiddenQueue, simN);
	freeQueue(Queue, simN);
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

	printf("%i\n", simN);
	printf("%i\n", seedR);
	printf("%f\n", lambda);
	printf("%i\n", threshED);
	printf("%i\n", conSwitch);
	printf("%f\n", alphC);
	printf("%i\n", timeSlice);

	srand(seedR);
	//FCFS
	int*** FCFSD = next_exp(lambda, simN, threshED);
	printHeader(FCFSD, simN, lambda);
	
	freeData(FCFSD, simN);
	srand(seedR);
	//SJF;
	int*** SJFD = next_exp(lambda, simN, threshED);
	sjf(SJFD, conSwitch, alphC, simN);
	freeData(SJFD, simN);
	

	srand(seedR);
	//SRT;
	int*** SRTD = next_exp(lambda, simN, threshED);
	freeData(SRTD, simN);

	srand(seedR);
	//RR;
	int*** RRD = next_exp(lambda, simN, threshED);
	freeData(RRD, simN);

	return 0;
}
